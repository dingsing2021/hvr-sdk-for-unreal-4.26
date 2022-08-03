// Fill out your copyright notice in the Description page of Project Settings.
#include "HuaweiVRCustomPresent.h"
#include "HuaweiVRSDK.h"

#include "HuaweiVRSDKAPI.h"
#include "HuaweiVRSDKJNI.h"

#if !WITH_EDITOR
#include "OpenGLDrvPrivate.h"
#include "OpenGLResources.h"
#endif

#if PLATFORM_ANDROID
extern void AndroidThunkCpp_KeepScreenOn(bool Enable);
#endif


// for Foveate  for凹形渲染
#if PLATFORM_ANDROID
FHWMobileHMDTextureSet::FHWMobileHMDTextureSet(//移动HMD 纹理
	class FOpenGLDynamicRHI* InGLRHI,
	GLenum InTarget,
	GLenum InAttachment,
	uint32 InSizeX,
	uint32 InSizeY,
	uint32 InSizeZ,
	uint32 InNumMips,
	uint32 InNumSamples,
	uint32 InNumSamplesTileMem,
	uint32 InArraySize,
	EPixelFormat InFormat,
	bool bInCubemap,
	bool bInAllocatedStorage,
	ETextureCreateFlags InFlags,
	uint8* InTextureRange,
	ETextureCreateFlags InTargetableTextureFlags
) : FOpenGLTexture2D(
	InGLRHI,
	0,
	InTarget,
	InAttachment,
	InSizeX,
	InSizeY,
	InSizeZ,
	InNumMips,
	InNumSamples,
	InNumSamplesTileMem,
	InArraySize,
	InFormat,
	bInCubemap,
	bInAllocatedStorage,
	InFlags,
	FClearValueBinding::Black)
{
	LOGI("HvrLog: FHWMobileHMDTextureSet::FHWMobileHMDTextureSet");
	FRHIResourceCreateInfo CreateInfo;
	bool bArrayTexture = (InArraySize > 1);
	for (int i = 0; i < HW_RENDERTARGETTXCTURENUM; ++i)
	{
		GLuint TextureID = 0;
		FOpenGL::GenTextures(1, &TextureID);

		const bool bSRGB = (InTargetableTextureFlags & TexCreate_SRGB) != 0;
		const FOpenGLTextureFormat& GLFormat = GOpenGLTextureFormats[InFormat];
		if (GLFormat.InternalFormat[bSRGB] == GL_NONE) {
			LOGI("GLFormat.InternalFormat[bSRGB] == GL_NONE");
		}

		FOpenGLContextState& ContextState = InGLRHI->GetContextStateForCurrentContext();
		// Make sure PBO is disabled
		InGLRHI->CachedBindPixelUnpackBuffer(ContextState, 0);

		// Use a texture stage that's not likely to be used for draws, to avoid waiting
		InGLRHI->CachedSetupTextureStage(ContextState, FOpenGL::GetMaxCombinedTextureImageUnits() - 1, InTarget, TextureID, 0, 1);

		if (bArrayTexture) {
			InTarget = GL_TEXTURE_2D_ARRAY;
			FOpenGL::TexStorage3D(InTarget, 1, GLFormat.InternalFormat[bSRGB], InSizeX, InSizeY, 2, GLFormat.Format, GLFormat.Type);
			mRenderTargetTextureRHIRef[i] = InGLRHI->RHICreateTexture2DArrayFromResource((EPixelFormat)PF_B8G8R8A8, InSizeX, InSizeY, 2, 1, InNumSamples, InNumSamplesTileMem, FClearValueBinding::Black, TextureID, InTargetableTextureFlags);
		}
		else {
			InTarget = GL_TEXTURE_2D;
			FOpenGL::TexStorage2D(InTarget, 1, GLFormat.SizedInternalFormat[bSRGB], InSizeX, InSizeY, GLFormat.Format, GLFormat.Type, InTargetableTextureFlags);

			//mRenderTargetTextureRHIRef[i]  mRenderTargetTextureRHIRefTy
			mRenderTargetTextureRHIRef[i] = InGLRHI->RHICreateTexture2DFromResource((EPixelFormat)PF_B8G8R8A8, InSizeX, InSizeY, 1, InNumSamples, InNumSamplesTileMem, FClearValueBinding::Black, TextureID, InTargetableTextureFlags);
		}
	}

	mRenderTargetTextureRHIRefIndex = 0;
}


FHWMobileHMDTextureSet* FHWMobileHMDTextureSet::CreateTexture2DSet(
	FOpenGLDynamicRHI* InGLRHI,
	GLenum InTarget,
	uint32 SizeX,
	uint32 SizeY,
	uint32 InNumSamples,
	uint32 InNumSamplesTileMem,
	uint32 InArraySize,
	EPixelFormat InFormat,
	ETextureCreateFlags InFlags,
	ETextureCreateFlags InTargetableTextureFlags,
	FTexture2DRHIRef& OutTargetableTexture,
	FTexture2DRHIRef& OutShaderResourceTexture)
{
	LOGI("HvrLog: FHWMobileHMDTextureSet::CreateTexture2DSet");
	FHWMobileHMDTextureSet* TextureSetProxy = new FHWMobileHMDTextureSet(InGLRHI, InTarget, GL_NONE, SizeX, SizeY, 0, 1, InNumSamples, InNumSamplesTileMem, InArraySize, InFormat, false, false, InFlags, nullptr, InTargetableTextureFlags);

	OutTargetableTexture = TextureSetProxy->GetTexture2D();  //mRenderTargetTextureRHIRefTy;  // 
	OutShaderResourceTexture = TextureSetProxy->GetTexture2D();

	return TextureSetProxy;

}

FHWMobileHMDTextureSet::~FHWMobileHMDTextureSet()
{
	LOGI("HvrLog: FHWMobileHMDTextureSet::~FHWMobileHMDTextureSet");
	this->SetMemorySize(0);
}

//选择下一个渲染目标
void FHWMobileHMDTextureSet::SwitchToNextElement()
{
	LOGI("HvrLog: FHWMobileHMDTextureSet::SwitchToNextElement");
	mRenderTargetTextureRHIRefIndex = ((mRenderTargetTextureRHIRefIndex + 1) % HW_RENDERTARGETTXCTURENUM);
	SetIndexAsCurrentRenderTarget();
}

//设置当前渲染目标，以下标的形式
void FHWMobileHMDTextureSet::SetIndexAsCurrentRenderTarget()
{
	LOGI("HvrLog: FHWMobileHMDTextureSet::SetIndexAsCurrentRenderTarget");
	Resource = GetRenderTargetResource();
}

//获取渲染目标资源(纹理)
GLuint FHWMobileHMDTextureSet::GetRenderTargetResource()
{
	LOGI("HvrLog: FHWMobileHMDTextureSet::GetRenderTargetResource");
	return *(GLuint*)mRenderTargetTextureRHIRef[mRenderTargetTextureRHIRefIndex]->GetNativeResource();
}

//获取黑色纹理资源（纹理）
GLuint FHWMobileHMDTextureSet::GetBlackTextureResource()
{
	LOGI("HvrLog: FHWMobileHMDTextureSet::GetBlackTextureResource");
	return *(GLuint*)BlackTexture->GetNativeResource();
}

//获得母板纹理资源（纹理）
GLuint FHWMobileHMDTextureSet::GetReticleTextureResource()
{
	LOGI("HvrLog: FHWMobileHMDTextureSet::GetReticleTextureResource");
	return *(GLuint*)ReticleTexture->GetNativeResource();
}
#endif
// for Foveate for中心渲染

//固定currentIndex = 0，意味着我们只会有一个rander target
FHWRenderTargets::FHWRenderTargets()
{
	CurrentIndex = 0;
	LOGI("HvrLog: FHWRenderTargets::FHWRenderTargets");
}

FHWRenderTargets::~FHWRenderTargets()
{
	LOGI("HvrLog: FHWRenderTargets::~FHWRenderTargets");
}

//初始化渲染目标
void FHWRenderTargets::InitRenderTarget(uint32 InSizeX, uint32 InSizeY)
{
	LOGI("HvrLog: FHWRenderTargets::InitRenderTarget");
    CurrentIndex = 0;

    FRHIResourceCreateInfo CreateInfo;
    FTexture2DRHIRef BufferedSRVRHI;

    IConsoleVariable* MobileMSAACVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MobileMSAA"));//查找控制台变量 MobileMSAA
    int msaa = MobileMSAACVar->GetInt();
	bool test = RHISupportsSeparateMSAAAndResolveTextures(GMaxRHIShaderPlatform);
	LOGI("HvrLog: FHWRenderTargets::InitRenderTarget msaa = %d, test = %d", msaa, test);

    RHICreateTargetableShaderResource2D(//创建有针对性的2D着色器
		InSizeX, 
		InSizeY, 
		PF_B8G8R8A8, 
		1, 
		TexCreate_None, 
		TexCreate_RenderTargetable,
        false, 
		true, 
		CreateInfo, 
		TextureRef, 
		BufferedSRVRHI, 
		msaa);

    LOGI("hvrlog: RHICreateTargetableShaderResource2D InSizeX=%u, InSizeY=%u", InSizeX, InSizeY);

    for (int i = 0; i < HW_RENDERTARGET_COUNT; i++) {
        SHWRenderTarget& tg = Target[i];
        tg.SensorState.Reset();
        for (int j = 0; j < HW_EYE_COUNT; j++) {
            RHICreateTargetableShaderResource2D(//再创建
				InSizeX / 2, 
				InSizeY, 
				PF_B8G8R8A8, //说明纹理格式是rgba
				1, 
				TexCreate_None,
                TexCreate_RenderTargetable, 
				false,
				true, 
				CreateInfo, 
				tg.EyeTexture[j], //生成目标纹理,输出图片之后可以通过这个纹理去拿到图
				BufferedSRVRHI, 
				msaa);

#if PLATFORM_ANDROID
			uint texID = *(uint*)tg.EyeTexture[j]->GetNativeResource();
			LOGI("hvrlog: RHICreateTexture2D[%d][%d] = %u, InSizeX=%u, InSizeY=%u, ", i, j, texID, InSizeX / 2, InSizeY);
#endif

        }
    }
}

//转到下一个渲染目标的下标
void FHWRenderTargets::SwitchToNextElement()
{
	LOGI("HvrLog: FHWRenderTargets::SwitchToNextElement");
    CurrentIndex = (CurrentIndex + 1) % HW_RENDERTARGET_COUNT;
}

//获得当前渲染目标
SHWRenderTarget& FHWRenderTargets::GetCurrentTarget()
{
	LOGI("HvrLog: FHWRenderTargets::GetCurrentTarget");
    return Target[CurrentIndex];
}

//获取上一个渲染目标
SHWRenderTarget& FHWRenderTargets::GetLastTarget()
{
	LOGI("HvrLog: FHWRenderTargets::GetLastTarget");
    int LastIndex = (CurrentIndex + HW_RENDERTARGET_COUNT - 1) % HW_RENDERTARGET_COUNT;
    return Target[LastIndex];
}

/* ************************************************************** */
FHuaweiVRCustomPresent::FHuaweiVRCustomPresent()//定制VR送显
    : m_RenderTargets(nullptr),  
	  mEyeType(0)
{
#if PLATFORM_ANDROID
	mGlTexIDTextureRefLeft = 0;
	mGlTexIDTextureRefRight = 0;
	mMainThreadRunning = true;
	mIsThreadOwnershipAcquired = false;
	mIsMobileMultiViewState = false;//是否选用中心区渲染，默认为false
	mOnApplicationStart = false;
	mOnresume = true;
#endif
	LOGI("HvrLog: FHuaweiVRCustomPresent::FHuaweiVRCustomPresent");
    Init();
}

FHuaweiVRCustomPresent::~FHuaweiVRCustomPresent()
{
    LOGI("HvrLog: FHuaweiVRCustomPresent::~FHuaweiVRCustomPresent");
    Shutdown();
    if (m_RenderTargets) {
        delete m_RenderTargets;
        m_RenderTargets = nullptr;
    }
}

//定制送显器的 初始化
void FHuaweiVRCustomPresent::Init()
{
	LOGI("HvrLog: FHuaweiVRCustomPresent::Init, IsInGameThread = %d, IsInRenderingThread = %d", IsInGameThread(), IsInRenderingThread());
#if !WITH_EDITOR
    auto GLRHI = static_cast<FOpenGLDynamicRHI*>(GDynamicRHI);
    GLRHI->SetCustomPresent(this);
#if PLATFORM_ANDROID
    HVR_SetInitVariables(FJavaWrapper::GameActivityThis);
    HVR_SendMessage(UNREAL_PLUGIN_FLAG, "true");

	bool bEnableMultiView = false;
	//是否开启中心区渲染取决于控制台参数 “vr.MobileMultiView”
	auto MobileMultiViewCVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("vr.MobileMultiView"));
	bEnableMultiView = GSupportsMobileMultiView && MobileMultiViewCVar && MobileMultiViewCVar->GetValueOnAnyThread() != 0;
#if ENGINE_MINOR_VERSION <25
	auto MobileMultiViewDirectCVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("vr.MobileMultiView.Direct"));
	bEnableMultiView = bEnableMultiView && MobileMultiViewDirectCVar && MobileMultiViewDirectCVar->GetValueOnAnyThread() != 0;
#endif
	/*
	static const auto CVarMobileMultiView = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("vr.MobileMultiView"));
	static const auto CVarMobileMultiViewDirect = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("vr.MobileMultiView.Direct"));
	static const auto CVarInstancedStereo = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("vr.InstancedStereo"));
	bool bIsMobileMultiViewEnabled = false;
	bIsMobileMultiViewEnabled = (CVarMobileMultiView && CVarMobileMultiView->GetValueOnAnyThread() != 0);
	bool bIsMobileMultiViewDirectEnabled = false;
	bIsMobileMultiViewDirectEnabled = (CVarMobileMultiViewDirect && CVarMobileMultiViewDirect->GetValueOnAnyThread() != 0);

	bool bIsInstancedStereoEnabled = false;
	bIsInstancedStereoEnabled = (CVarInstancedStereo && CVarInstancedStereo->GetValueOnAnyThread() != 0);

	mIsMobileMultiViewState = bIsMobileMultiViewEnabled && bIsMobileMultiViewDirectEnabled;
	*/
	mIsMobileMultiViewState = bEnableMultiView;
	LOGI("HvrLog: FHuaweiVRCustomPresent::Init mIsMobileMultiViewState1 = %d", mIsMobileMultiViewState);

#endif
#endif
}

//中断
void FHuaweiVRCustomPresent::Shutdown()
{
	LOGI("HvrLog: FHuaweiVRCustomPresent::Shutdown");
#if !WITH_EDITOR
    auto GLRHI = static_cast<FOpenGLDynamicRHI*>(GDynamicRHI);
    GLRHI->SetCustomPresent(nullptr);
#if PLATFORM_ANDROID
    HVR_ShutdownRenderThread();//通知SDK层停止渲染线程
#endif
#endif
}

// Called when viewport is resized.
void FHuaweiVRCustomPresent::OnBackBufferResize()
{
	LOGI("HvrLog: FHuaweiVRCustomPresent::OnBackBufferResize");
}

//由主线程通知， 是否还运行
void FHuaweiVRCustomPresent::MainThreadRunningNotify(bool running)
{
	LOGI("HvrLog: FHuaweiVRCustomPresent::MainThreadRunningNotify %d", running);
#if PLATFORM_ANDROID
    mMainThreadRunning = running;
	if(running) {
		mOnresume = running;
	}
#endif
}

// @param InOutSyncInterval - in out param, indicates if vsync is on (>0) or off (==0).  表示vsync是否来到
// @return    true if normal Present should be performed 执行; false otherwise. If it returns
// true, then InOutSyncInterval could be modified to switch between VSync/NoVSync for the normal Present.
bool FHuaweiVRCustomPresent::Present(int32& InOutSyncInterval)
{
	LOGI("HvrLog: FHuaweiVRCustomPresent::Present");
    // check(IsInRenderingThread());
#if PLATFORM_ANDROID
	if (mOnresume) {
		AndroidThunkCpp_KeepScreenOn(true);
		LOGI("hvrlog: FHuaweiVRCustomPresent::FinishRendering HVR_Resume %d", gettid());
		HVR_Resume();  // 首次开始送显的时候进行渲染线程的resume
		mOnresume = false;
		mIsThreadOwnershipAcquired = true;
	}
    if (mIsThreadOwnershipAcquired) {//是否已经获取线程所有权
        // GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Green, DebugVersion, true, FVector2D(5, 5));
        FinishRendering();
    }
#endif
    // Since the timing of the system call OnReleaseThreadOwnership is too late,
    // it will cause the Vsync initialization and destruction timing to be disorderly,
    // so it is modified to destroy the rendering thread when the Activity pauses,
    // and try to ensure that the Vsync timing is correct.
	//由于系统调用OnReleaseThreadOwnership的时间太晚，
	//这会导致Vsync初始化和销毁时序混乱，
	//因此修改为在Activity暂停时销毁渲染线程
	//并尝试确保Vsync时序正确。
#if PLATFORM_ANDROID
    if (!mMainThreadRunning) {//这是退出时调用
        LOGI("hvrlog: FHuaweiVRCustomPresent::Present OnReleaseThreadOwnership");
        OnReleaseThreadOwnership();
    }
#endif
    // Note: true causes normal swapbuffers(), false prevents normal swapbuffers()
    return false;
}

//会调用这个函数三次
// Called when rendering thread is acquired
void FHuaweiVRCustomPresent::OnAcquireThreadOwnership()//获取线程所有权之后执行的操作
{
	LOGI("HvrLog: FHuaweiVRCustomPresent::OnAcquireThreadOwnership, IsInGameThread = %d, IsInRenderingThread = %d", IsInGameThread(), IsInRenderingThread());
#if PLATFORM_ANDROID
    AndroidThunkCpp_KeepScreenOn(true);
#endif
    // LOGI("FHuaweiVRCustomPresent::OnAcquireThreadOwnership");
    check(IsInRenderingThread());
#if PLATFORM_ANDROID
    if (!mOnApplicationStart) {
        if (HVR_SensorGetContainer() < 0) {
            FinishActivity();
        }
        LOGI("FHuaweiVRCustomPresent::OnAcquireThreadOwnership HVR_InitRenderThread %d", gettid());
		if (mIsMobileMultiViewState) {
			LOGI("HVR_InitRenderFoveatedThread");
			HVR_InitRenderFoveatedThread();//初始化中心渲染线程
		} else {
			LOGI("HVR_InitRenderThread");//初始化普通渲染线程，当前是只开启了普通渲染模式
			HVR_InitRenderThread();
		}
        mOnApplicationStart = true;
		// mOnresume = false;

    } else {
#if PLATFORM_ANDROID
        // LOGI("FHuaweiVRCustomPresent::OnAcquireThreadOwnership HVR_Resume %d", gettid());
        // HVR_Resume();
#endif
    }
#endif
#if PLATFORM_ANDROID
	mIsThreadOwnershipAcquired = true;//表示拿到了线程所有权
#endif

    
}

// 422
// Called when rendering thread is released
//这个函数被调用了4次，最后一次可能是真的释放时调用
//第一次进来，已经取得线程权；第2、3次进来没有取得线程权；最后一次（结束）时进来也没有取得线程权。（这一系列操作我不明白）
void FHuaweiVRCustomPresent::OnReleaseThreadOwnership()//当渲染线程要被释放所有权时所执行的操作
{
	LOGI("HvrLog: FHuaweiVRCustomPresent::OnReleaseThreadOwnership, IsInGameThread = %d, IsInRenderingThread = %d", IsInGameThread(), IsInRenderingThread());
	// check(IsInRenderingThread());//422

	// if (!IsInRenderingThread()) {
	// 	LOGI("HvrLog: OnReleaseThreadOwnership-Not inRenderingThread");
	// 	return;
	// }
#if PLATFORM_ANDROID
    if (mIsThreadOwnershipAcquired) {//如果进来没有取得线程权，则暂停

        LOGI("HvrLog: FHuaweiVRCustomPresent::OnReleaseThreadOwnership HVR_Pause %d", gettid());
        HVR_Pause();//这里应该会暂停，直到mIsThreadOwnershipAcquired为真为止
		// mOnresume = true;

    }
	LOGI("HvrLog: FHuaweiVRCustomPresent::mIsThreadOwnershipAcquired = %d", (int)mIsThreadOwnershipAcquired);
#endif

#if PLATFORM_ANDROID
	mIsThreadOwnershipAcquired = false;
#endif

#if PLATFORM_ANDROID
    AndroidThunkCpp_KeepScreenOn(false);
#endif
}

//分配、分摊渲染目标的纹理，这个函数一共被调用了两次
bool FHuaweiVRCustomPresent::AllocateRenderTargetTexture(uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips,
	ETextureCreateFlags Flags, ETextureCreateFlags TargetableTextureFlags, FTexture2DRHIRef& OutTargetableTexture,
    FTexture2DRHIRef& OutShaderResourceTexture, uint32 NumSamples)
{
	LOGI("HvrLog: FHuaweiVRCustomPresent::AllocateRenderTargetTexture");

    // Flags |= TargetableTextureFlags; // 422
	// Flags = (ETextureCreateFlags) (Flags | TargetableTextureFlags);

    LOGI("HvrLog: Allocated a new swap texture set (size %u x %u)", SizeX, SizeY);

    if (m_RenderTargets == nullptr) {//如果渲染目标是空，则创建
        m_RenderTargets = new FHWRenderTargets();
    }
#if PLATFORM_ANDROID
	if (mIsMobileMultiViewState) {
		LOGI("HvrLog: AllocateRenderTargetTexture mIsMobileMultiViewState");
		IConsoleVariable* MobileMSAACVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MobileMSAA"));
		int msaa = MobileMSAACVar->GetInt();
		GLenum Target = GL_TEXTURE_2D_ARRAY;
		uint32 Layers = 2;
		auto OpenGLDynamicRHI = static_cast<FOpenGLDynamicRHI*>(GDynamicRHI);
		check(OpenGLDynamicRHI);
		mTextureSet = TRefCountPtr<FHWMobileHMDTextureSet>(FHWMobileHMDTextureSet::CreateTexture2DSet(OpenGLDynamicRHI, Target, SizeX, SizeY, NumSamples, msaa, Layers, EPixelFormat(Format), 
		TexCreate_RenderTargetable | TexCreate_ShaderResource, TargetableTextureFlags, OutTargetableTexture, OutShaderResourceTexture));

	} else {//现在是没有开启中心渲染的
		LOGI("HvrLog: AllocateRenderTargetTexture mIsMobileMultiViewState not");
		m_RenderTargets->InitRenderTarget(SizeX, SizeY);//渲染目标的宽度SizeX实际上是眼睛宽度的两倍，即双宽，由HVR_SetEyeParms(SizeX / 2, SizeY)可知

		OutTargetableTexture = m_RenderTargets->TextureRef;
		OutShaderResourceTexture = m_RenderTargets->TextureRef;
	}
#endif

    if (!OutTargetableTexture.IsValid()) {//判断OutTargetableTexture（输出目标纹理）是否有效  Valid-有效
        UE_LOG(LogHuaweiVRSDK, Error, TEXT("OutTargetableTexture is not valid "));
    }

    if (!OutShaderResourceTexture.IsValid()) {//判断 输出着色器资源纹理 是否有效， Valid-有效
        UE_LOG(LogHuaweiVRSDK, Error, TEXT("OutShaderResourceTexture is not valid "));
    }

#if PLATFORM_ANDROID
    HVR_SetEyeParms(SizeX / 2, SizeY);
#endif

	LOGI("HvrLog: AllocateRenderTargetTexture OUT;");

    return true;
}

//开始渲染
void FHuaweiVRCustomPresent::BeginRendering(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) { LOGI("HvrLog: FHuaweiVRCustomPresent::BeginRendering"); }

//结束渲染(重要函数，每显示一帧就调用到这个函数，这个函数被Unreal引擎代码调用Present时调用到)
void FHuaweiVRCustomPresent::FinishRendering()
{
	LOGI("HvrLog: FHuaweiVRCustomPresent::FinishRendering");
    if (m_RenderTargets == NULL)
        return;

#if PLATFORM_ANDROID
	if (mIsMobileMultiViewState && mTextureSet == NULL) {//如果开启了中心渲染但是纹理是空的，是属于异常，不做任何处理
		LOGI("mTextureSet == NULL");
		return;
	}
#endif

	//m_RenderTargets->SwitchToNextElement();//指向下一帧
	//ConditionalUpdateCache();//发一个信号量，更新一次render target的位姿
    SHWRenderTarget& RenderTarget = m_RenderTargets->GetCurrentTarget();//获得当前渲染target

#if PLATFORM_ANDROID

	if (mIsMobileMultiViewState) {
		uint mGlTexIDTextureRefTest = mTextureSet->GetRenderTargetResource();//test

		if (mEyeType == 0) {
			mGlTexIDTextureRefLeft = mTextureSet->GetRenderTargetResource();
			HVR_CameraEndFrameFoveated(mEyeType, (int)mGlTexIDTextureRefLeft, 0);//这里的camera其实说的是人眼
			mTextureSet->SwitchToNextElement();
			mEyeType = 1;
			return;
		}
		if (mEyeType == 1) {
			mGlTexIDTextureRefRight = mTextureSet->GetRenderTargetResource();
			HVR_CameraEndFrameFoveated(mEyeType, (int)mGlTexIDTextureRefRight, 0);
			mEyeType = 0;
		}

		mTextureSet->SwitchToNextElement();
		
	} else {
		uint glTexID = *(uint*)RenderTarget.EyeTexture[0]->GetNativeResource();

		HVR_CameraEndFrame(0, (int)glTexID, 0);

		glTexID = *(uint*)RenderTarget.EyeTexture[1]->GetNativeResource();
		HVR_CameraEndFrame(1, (int)glTexID, 0);
	}

    HVR_TimeWarpEvent(RenderTarget.SensorState.twIdx);//这里调用TIME WARP以显示该帧

#endif
    m_RenderTargets->SwitchToNextElement();//指向下一帧
	allowUpdatePose = true;
    //ConditionalUpdateCache();//发一个信号量，更新一次render target的位姿
}

void FHuaweiVRCustomPresent::ConditionalUpdateCache()
{
	LOGI("HvrLog: FHuaweiVRCustomPresent::ConditionalUpdateCache");
	if (m_RenderTargets == nullptr) {
        return;
    }
    SHWRenderTarget& RenderTarget = m_RenderTargets->GetCurrentTarget();//获取当前渲染目标

    SensorState& st = RenderTarget.SensorState;//准备设置渲染目标中的传感器状态（位姿）

#if PLATFORM_ANDROID
	//根据SDK代码，这里必须先调用HVR_GetSensorState再调用HVR_GetCameraPositionOrientation
	//因为HVR_GetSensorState会刷新传感器数据
    HVR_GetSensorState(false, st.w, st.x, st.y, st.z, st.fov, st.twIdx);//获取传感器状态， wxyz表示头盔的旋转角

	float ow = 1.0f, ox = 0.0f, oy = 0.0f, oz = 0.0f;
	HVR_GetCameraPositionOrientation(st.px, st.py, st.pz, ox, oy, oz, ow);//再获取摄像机位姿

	FHuaweiVRSDK* huaweiVRSDK = FHuaweiVRSDK::GetHuaweiVRSDK();

	st.px *= huaweiVRSDK->GetWorldToMetersScale();//再乘以一个世界比例
	st.py *= huaweiVRSDK->GetWorldToMetersScale();
	st.pz *= huaweiVRSDK->GetWorldToMetersScale();

#endif
}
/*
void FHuaweiVRCustomPresent::UpdateViewport(const class FViewport& Viewport, class FRHIViewport* InViewportRHI) {
    check(IsInGameThread());
    check(ViewportRHI);

    if (m_RenderTargets) {
        ViewportRHI->SetCustomPresent(this);
    }
} */

bool FHuaweiVRCustomPresent::NeedsNativePresent()//需要本机呈现
{
	LOGI("HvrLog: FHuaweiVRCustomPresent::NeedsNativePresent");
    return true;
}