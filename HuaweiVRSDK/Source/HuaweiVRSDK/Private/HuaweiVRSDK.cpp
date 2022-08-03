#include "HuaweiVRSDK.h"
#include "Misc/App.h"
#include "Misc/CoreDelegates.h"
#include "Modules/ModuleManager.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"
#include "Engine/LocalPlayer.h"
#include "IHuaweiVRSDKPlugin.h"
#include "SceneRendering.h"
#include "PostProcess/PostProcessHMD.h"
#include "GameFramework/WorldSettings.h"
#include "Slate/SceneViewport.h"
#include "HuaweiVRSDKMessageTable.h"
#include "IXRTrackingSystem.h"

#define HUAWEI_RT_WIDTH 1080
#define HUAWEI_RT_HEIGHT 1080


#define HUAWEI_RT_WIDTH_MULTIVIEW_L 720
#define HUAWEI_RT_HEIGHT_MULTIVIEW_L 720

#define HUAWEI_RT_WIDTH_MULTIVIEW_M 1080
#define HUAWEI_RT_HEIGHT_MULTIVIEW_M 1080

#define HUAWEI_RT_WIDTH_MULTIVIEW_H 1280
#define HUAWEI_RT_HEIGHT_MULTIVIEW_H 1280

FThreadSafeBool FHuaweiVRSDK::RenderFPSChanged = false;
int FHuaweiVRSDK::RenderFrameRate = 60;
int FHuaweiVRSDK::RenderSizeX = HUAWEI_RT_WIDTH;
int FHuaweiVRSDK::RenderSizeY = HUAWEI_RT_HEIGHT;
bool FHuaweiVRSDK::IsMultiviewState = false;

static const FString HuaweiVRSDKUnrealVersion = TEXT("HuaweiVR Unreal SDK Version: v3.5.0.72");

// ---------------------------------------------------
// HuaweiVRSDK Plugin Implementation
// ---------------------------------------------------

class FHuaweiVRSDKPlugin : public IHuaweiVRSDKPlugin {
    /* * IHeadMountedDisplayModule implementation */
    virtual TSharedPtr<class IXRTrackingSystem, ESPMode::ThreadSafe> CreateTrackingSystem() override;
    FString GetModuleKeyName() const override
    {
        return FString(TEXT("HuaweiVRSDK"));
    }
};

IMPLEMENT_MODULE(FHuaweiVRSDKPlugin, HuaweiVRSDK)//IMPLEMENT_MODULE这个宏，真正把我们的插件关联上

TSharedPtr<class IXRTrackingSystem, ESPMode::ThreadSafe> FHuaweiVRSDKPlugin::CreateTrackingSystem()
{
    LOGI("HvrLog: FHuaweiVRSDKPlugin::CreateTrackingSystem");
    auto HuaweiVRSDK = FSceneViewExtensions::NewExtension<FHuaweiVRSDK>();
    if (HuaweiVRSDK->IsInitialized()) {
        return HuaweiVRSDK;
    }
    return nullptr;
}

FHuaweiVRSDK* FHuaweiVRSDK::GetHuaweiVRSDK()
{
    LOGI("HvrLog: FHuaweiVRSDKPlugin::GetHuaweiVRSDK");
    if (GEngine && GEngine->XRSystem.IsValid()) {
        IHeadMountedDisplay* HMDDevice = GEngine->XRSystem->GetHMDDevice();
        if (HMDDevice) {
            return static_cast<FHuaweiVRSDK*>(HMDDevice);
        }
    }
    return nullptr;
}

float FHuaweiVRSDK::GetWorldToMetersScale() const
{
    LOGI("HvrLog: FHuaweiVRSDKPlugin::GetWorldToMetersScale");
    return GWorld ? GWorld->GetWorldSettings()->WorldToMeters : 100.0f;
}

// ---------------------------------------------------
// HuaweiVRSDK IHeadMountedDisplay Implementation
// ---------------------------------------------------

bool FHuaweiVRSDK::IsHMDEnabled() const
{
    LOGI("HvrLog: FHuaweiVRSDK::IsHMDEnabled");
    return true;
}

void FHuaweiVRSDK::EnableHMD(bool enable) { LOGI("HvrLog: FHuaweiVRSDK::EnableHMD"); }

//获取HMD监视器信息
bool FHuaweiVRSDK::GetHMDMonitorInfo(MonitorInfo& MonitorDesc)
{
    LOGI("HvrLog: FHuaweiVRSDK::GetHMDMonitorInfo");
    MonitorDesc.MonitorName = "";
    MonitorDesc.MonitorId = 0;
    MonitorDesc.DesktopX = MonitorDesc.DesktopY = MonitorDesc.ResolutionX = MonitorDesc.ResolutionY = 0;
    return false;
}

//获取视场角
void FHuaweiVRSDK::GetFieldOfView(float& OutHFOVInDegrees, float& OutVFOVInDegrees) const
{
    LOGI("HvrLog: FHuaweiVRSDK::GetFieldOfView");
    OutHFOVInDegrees = 0.0f;
    OutVFOVInDegrees = 0.0f;
}

//枚举设备
bool FHuaweiVRSDK::EnumerateTrackedDevices(TArray<int32>& OutDevices, EXRTrackedDeviceType Type)
{
    LOGI("HvrLog: FHuaweiVRSDK::EnumerateTrackedDevices");
    if (Type == EXRTrackedDeviceType::Any || Type == EXRTrackedDeviceType::HeadMountedDisplay) {
        OutDevices.Add(IXRTrackingSystem::HMDDeviceId);//其实我们就一个设备，所以仅add这个就行
        return true;
    }
    return false;
}

void FHuaweiVRSDK::SetInterpupillaryDistance(float NewInterpupillaryDistance) { LOGI("HvrLog: FHuaweiVRSDK::SetInterpupillaryDistance"); }

//取瞳孔间距
float FHuaweiVRSDK::GetInterpupillaryDistance() const
{
    LOGI("HvrLog: FHuaweiVRSDK::GetInterpupillaryDistance");
    return 0.064f;
}

//取HMD方向和位置
void FHuaweiVRSDK::GetHMDOrientationAndPosition(FQuat& CurrentOrientation, FVector& CurrentPosition)
{
    LOGI("HvrLog: FHuaweiVRSDK::GetHMDOrientationAndPosition");
    // very basic.  no head model, no prediction, using debuglocalplayer
    // ULocalPlayer* Player = GEngine->GetDebugLocalPlayer();

    CurrentOrientation = CurHmdOrientation;
    CurrentPosition = CurHmdPosition;
	//if (CustomPresent && CustomPresent->m_RenderTargets) {
 //       //CustomPresent->ConditionalUpdateCache();//wxq:  刷新传感器数据

 //       //SHWRenderTarget& RenderTarget = CustomPresent->m_RenderTargets->GetCurrentTarget();
 //       //SensorState& st = RenderTarget.SensorState;

	//	//LOGI("CurrentPosition:%f %f %f", CurrentPosition.X, CurrentPosition.Y, CurrentPosition.Z);
 //   } else {
 //       LOGW("GetHMDOrientationAndPosition failed");
 //       CurrentOrientation = FQuat(0.0f, 0.0f, 0.0f, 1.0f);
 //       CurrentPosition = FVector(0.0f, 0.0f, 0.0f);
 //   }
}

//取end游戏帧
bool FHuaweiVRSDK::OnEndGameFrame(FWorldContext& WorldContext)
{
    mGameFrameEnded = true;
    return false;
}

//明星函数： 获取当前姿态，如果没有覆写，则使用UE自定义的,姿态就是 位置&方向
//渲染一帧会调用7次这个函数
bool FHuaweiVRSDK::GetCurrentPose(int32 DeviceId, FQuat& CurrentOrientation, FVector& CurrentPosition)
{
    LOGI("HvrLog: FHuaweiVRSDK::GetCurrentPose");
    if (DeviceId != IXRTrackingSystem::HMDDeviceId) {//如果设备ID不对，返回false
        LOGI("hvrlog: DeviceId != IXRTrackingSystem::HMDDeviceId ");
        return false;
    }
    
    if (CustomPresent && CustomPresent->m_RenderTargets) {
        if (CustomPresent->allowUpdatePose) {
            CustomPresent->ConditionalUpdateCache();//wxq:  刷新传感器数据
            CustomPresent->allowUpdatePose = false;
        }
        
        SHWRenderTarget& RenderTarget = CustomPresent->m_RenderTargets->GetCurrentTarget();
        SensorState& st = RenderTarget.SensorState;
        CurrentOrientation = BaseOrientation * FQuat(st.z, st.x, st.y, st.w);
        CurrentPosition = FVector(st.pz, st.px, st.py) - BasePos;
    }
    else {
        LOGI("hvrlog: FHuaweiVRSDK::GetCurrentPose = 0");
        CurrentOrientation = FQuat::Identity;
        CurrentPosition = FVector::ZeroVector;
    }
    
    //wxq:
    //if (IsInRenderingThread() || IsInGameThread()) {
    //    GetHMDOrientationAndPosition(CurHmdOrientation, CurHmdPosition);
    //}
    //wxq: end

	//CurrentOrientation = CurHmdOrientation;
	//CurrentPosition = CurHmdPosition;
	
    return true;
}

//是否启用了ChromaAb校正
bool FHuaweiVRSDK::IsChromaAbCorrectionEnabled() const
{
    LOGI("HvrLog: FHuaweiVRSDK::IsChromaAbCorrectionEnabled");
    return false;
}

//重置方向和位姿
void FHuaweiVRSDK::ResetOrientationAndPosition(float yaw)
{
    LOGI("HvrLog: FHuaweiVRSDK::ResetOrientationAndPosition");
    ResetOrientation(yaw);
    ResetPosition();
}

//重置旋转角
void FHuaweiVRSDK::ResetOrientation(float Yaw)
{
    LOGI("HvrLog: FHuaweiVRSDK::ResetOrientation");
#if PLATFORM_ANDROID
    HVR_ResetSensorOrientation();//重置传感器的旋转角
#endif
}

//重置位置 useless
void FHuaweiVRSDK::ResetPosition() { LOGI("HvrLog: FHuaweiVRSDK::ResetPosition"); }

//设置基本旋转角
void FHuaweiVRSDK::SetBaseRotation(const FRotator& BaseRot) {
    LOGI("HvrLog: FHuaweiVRSDK::SetBaseRotation");
}

//获取基本旋转角
FRotator FHuaweiVRSDK::GetBaseRotation() const
{
    LOGI("HvrLog: FHuaweiVRSDK::GetBaseRotation");
    return FRotator::ZeroRotator;
}

//设置基本方向（没有调用）
void FHuaweiVRSDK::SetBaseOrientation(const FQuat& BaseOrient) {
    LOGI("HvrLog: FHuaweiVRSDK::SetBaseOrientation");
	BaseOrientation = BaseOrient;
}

//获取基本方向(没有调用)
FQuat FHuaweiVRSDK::GetBaseOrientation() const
{
    LOGI("HvrLog: FHuaweiVRSDK::GetBaseOrientation");
    return BaseOrientation;
}

//设置基本位置
void FHuaweiVRSDK::SetBasePosition(const FVector& BasePosition) {
    LOGI("HvrLog: FHuaweiVRSDK::SetBasePosition");
	BasePos = BasePosition;
	//LOGI("SetBasePosition BasePos:%f %f %f", BasePosition.X, BasePosition.Y, BasePosition.Z);
}

//获取基本位置
FVector FHuaweiVRSDK::GetBasePosition() const
{
    LOGI("HvrLog: FHuaweiVRSDK::GetBasePosition");
    return BasePos;
}

//绘制方向网格 由渲染线程调用
void FHuaweiVRSDK::DrawDistortionMesh_RenderThread(struct FRenderingCompositePassContext& Context,
    const FIntPoint& TextureSize)
{
    LOGI("HvrLog: FHuaweiVRSDK::DrawDistortionMesh_RenderThread");
    float ClipSpaceQuadZ = 0.0f;
    FMatrix QuadTexTransform = FMatrix::Identity;
    FMatrix QuadPosTransform = FMatrix::Identity;
    const FViewInfo& View = Context.View;
    const FIntRect SrcRect = View.UnscaledViewRect;

    FRHICommandListImmediate& RHICmdList = Context.RHICmdList;
    const FSceneViewFamily& ViewFamily = *(View.Family);
    FIntPoint ViewportSize = ViewFamily.RenderTarget->GetSizeXY();
    RHICmdList.SetViewport(0, 0, 0.0f, ViewportSize.X, ViewportSize.Y, 1.0f);

    static const uint32 NumVerts = 8;
    static const uint32 NumTris = 4;

    static const FDistortionVertex Verts[8] = {
        // left eye
        { FVector2D(-0.9f, -0.9f), FVector2D(0.0f, 1.0f), FVector2D(0.0f, 1.0f), FVector2D(0.0f, 1.0f), 1.0f, 0.0f },
        { FVector2D(-0.1f, -0.9f), FVector2D(0.5f, 1.0f), FVector2D(0.5f, 1.0f), FVector2D(0.5f, 1.0f), 1.0f, 0.0f },
        { FVector2D(-0.1f, 0.9f), FVector2D(0.5f, 0.0f), FVector2D(0.5f, 0.0f), FVector2D(0.5f, 0.0f), 1.0f, 0.0f },
        { FVector2D(-0.9f, 0.9f), FVector2D(0.0f, 0.0f), FVector2D(0.0f, 0.0f), FVector2D(0.0f, 0.0f), 1.0f, 0.0f },
        // right eye
        { FVector2D(0.1f, -0.9f), FVector2D(0.5f, 1.0f), FVector2D(0.5f, 1.0f), FVector2D(0.5f, 1.0f), 1.0f, 0.0f },
        { FVector2D(0.9f, -0.9f), FVector2D(1.0f, 1.0f), FVector2D(1.0f, 1.0f), FVector2D(1.0f, 1.0f), 1.0f, 0.0f },
        { FVector2D(0.9f, 0.9f), FVector2D(1.0f, 0.0f), FVector2D(1.0f, 0.0f), FVector2D(1.0f, 0.0f), 1.0f, 0.0f },
        { FVector2D(0.1f, 0.9f), FVector2D(0.5f, 0.0f), FVector2D(0.5f, 0.0f), FVector2D(0.5f, 0.0f), 1.0f, 0.0f },
    };

    FRHIResourceCreateInfo CreateInfo;
    FVertexBufferRHIRef VertexBufferRHI =
        RHICreateVertexBuffer(sizeof(FDistortionVertex) * 8, BUF_Volatile, CreateInfo);
    void* VoidPtr = RHILockVertexBuffer(VertexBufferRHI, 0, sizeof(FDistortionVertex) * 8, RLM_WriteOnly);
    FPlatformMemory::Memcpy(VoidPtr, Verts, sizeof(FDistortionVertex) * 8);
    RHIUnlockVertexBuffer(VertexBufferRHI);

    static const uint16 Indices[12] = { /* Left */ 0, 1, 2, 0, 2, 3, /* Right */ 4, 5, 6, 4, 6, 7 };

    FIndexBufferRHIRef IndexBufferRHI =
        RHICreateIndexBuffer(sizeof(uint16), sizeof(uint16) * 12, BUF_Volatile, CreateInfo);
    void* VoidPtr2 = RHILockIndexBuffer(IndexBufferRHI, 0, sizeof(uint16) * 12, RLM_WriteOnly);
    FPlatformMemory::Memcpy(VoidPtr2, Indices, sizeof(uint16) * 12);
    RHIUnlockIndexBuffer(IndexBufferRHI);

    RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
    // RHICmdList.DrawIndexedPrimitive(IndexBufferRHI, PT_TriangleList, 0, 0, NumVerts, 0, NumTris, 1); // 422 usage
    RHICmdList.DrawIndexedPrimitive(IndexBufferRHI, 0, 0, NumVerts, 0, NumTris, 1);

    IndexBufferRHI.SafeRelease();
    VertexBufferRHI.SafeRelease();
}

//获取Idea（理想的）渲染目标尺寸
FIntPoint FHuaweiVRSDK::GetIdealRenderTargetSize() const
{
     LOGI("HvrLog: GetIdealRenderTargetSize RenderSizeX = %d, RenderSizeY = %d", RenderSizeX, RenderSizeY);
    return FIntPoint(RenderSizeX * 2, RenderSizeY); // 1552
}

//是否立体 使能
bool FHuaweiVRSDK::IsStereoEnabled() const
{
    LOGI("HvrLog: FHuaweiVRSDK::IsStereoEnabled");
    return true;
}

//立体使能
bool FHuaweiVRSDK::EnableStereo(bool stereo)
{
    LOGI("HvrLog: FHuaweiVRSDK::EnableStereo");
    return true;
}

//调整视口矩形（被调用就意味着，多通道渲染在切换视口）
void FHuaweiVRSDK::AdjustViewRect(EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const
{ 
	SizeX = RenderSizeX * 2;
	SizeY = RenderSizeY;
#if PLATFORM_ANDROID
	if (!CustomPresent->mIsMobileMultiViewState) {
		SizeX = SizeX / 2;
	}
#endif 

    if (StereoPass == eSSP_RIGHT_EYE) {//如果是渲染右眼，则加一个图宽偏移
        X += SizeX;
    }
    LOGI("HvrLog: FHuaweiVRSDK::AdjustViewRect (x, y):(%d, %d) (SizeX, SizeY):(%u, %u)", X, Y, SizeX, SizeY);
}

//计算立体视图的偏移，用到姿态（被多次UE调用）
void FHuaweiVRSDK::CalculateStereoViewOffset(const enum EStereoscopicPass StereoPassType, FRotator& ViewRotation,
    const float WorldToMeters, FVector& ViewLocation)//这个是继承了
{
    LOGI("HvrLog: FHuaweiVRSDK::, StereoPassType = %d", StereoPassType);
    //LOGI("HvrLog: FHuaweiVRSDK::StereoPassType1 = %d, Do[GetHMDOrientationAndPosition]", StereoPassType);
 //   if (StereoPassType == eSSP_LEFT_EYE) {
 //       GetHMDOrientationAndPosition(CurHmdOrientation, CurHmdPosition);//获取HMD 方向和位置，更新hmd位姿
	//}
    //if (StereoPassType == 0) {
    //    GetCurrentPose(HMDDeviceId, CurHmdOrientation, CurHmdPosition);//获取HMD 方向和位置，更新hmd位姿
    //}
	FHeadMountedDisplayBase::CalculateStereoViewOffset(StereoPassType, ViewRotation, WorldToMeters, ViewLocation);
	//LOGI("CalculateStereoViewOffset ViewLocation:%f %f %f", ViewLocation.X, ViewLocation.Y, ViewLocation.Z);
}

//获取相对眼睛姿势
bool  FHuaweiVRSDK::GetRelativeEyePose(int32 DeviceId, EStereoscopicPass StereoPassType, FQuat& OutOrientation, FVector& OutPosition)
{
    LOGI("HvrLog: FHuaweiVRSDK::GetRelativeEyePose");
	OutOrientation = FQuat::Identity;//方向固定
	OutPosition = FVector::ZeroVector;
	if (DeviceId == HMDDeviceId && (StereoPassType != eSSP_FULL)) //(StereoPassType == eSSP_LEFT_EYE || StereoPassType == eSSP_RIGHT_EYE)
	{
		const float EyeOffset = (GetInterpupillaryDistance() * 0.5f) * GetWorldToMetersScale();
		const float PassOffset = (StereoPassType == eSSP_LEFT_EYE) ? -EyeOffset : EyeOffset;
		OutPosition = FVector(0, PassOffset, 0);
		return true;
	}
	else
	{
		return false;
	}
}

//获取立体投影矩阵（立体投影）,这个函数被多次调用
FMatrix FHuaweiVRSDK::GetStereoProjectionMatrix(const enum EStereoscopicPass StereoPassType) const
{
    LOGI("HvrLog: FHuaweiVRSDK::GetStereoProjectionMatrix");
#if PLATFORM_ANDROID
    float fFOV = 95.0f;//fov固定95，
    HVR_GetFOVAngle(fFOV);//从下层获取FOV，下层的FOV也是写死的95

    //指表示开启了中心区渲染，eSSP_LEFT_EYE应该是表示左眼执行中心区渲染
	if (CustomPresent->mIsMobileMultiViewState && (StereoPassType == eSSP_LEFT_EYE)) {//后条件成立
		LOGI("vrlog: CustomPresent->mIsMobileMultiViewState && (StereoPassType == eSSP_LEFT_EYE)");
		fFOV = fFOV / 2.0f;
	}
    LOGI("vrlog: StereoPassType == %d", StereoPassType);

    const float ProjectionCenterOffset = 0.0f;
    const float PassProjectionOffset = (StereoPassType == eSSP_LEFT_EYE) ? ProjectionCenterOffset : -ProjectionCenterOffset;

    const float HalfFov = FMath::DegreesToRadians(fFOV) / 2.f;
    const float InWidth = HUAWEI_RT_WIDTH;
    const float InHeight = HUAWEI_RT_HEIGHT;
    const float XS = 1.0f / tan(HalfFov);
    const float YS = InWidth / tan(HalfFov) / InHeight;

    const float InNearZ = GNearClippingPlane;
    return FMatrix(FPlane(XS, 0.0f, 0.0f, 0.0f), FPlane(0.0f, YS, 0.0f, 0.0f), FPlane(0.0f, 0.0f, 0.0f, 1.0f),
        FPlane(0.0f, 0.0f, InNearZ, 0.0f)) * FTranslationMatrix(FVector(PassProjectionOffset, 0, 0));
#else
    const float ProjectionCenterOffset = 0.151976421f;
    const float PassProjectionOffset =
        (StereoPassType == eSSP_LEFT_EYE) ? ProjectionCenterOffset : -ProjectionCenterOffset;

    const float HalfFov = 2.19686294f / 2.f;
    const float InWidth = 640.f;
    const float InHeight = 480.f;
    const float XS = 1.0f / tan(HalfFov);
    const float YS = InWidth / tan(HalfFov) / InHeight;

    const float InNearZ = GNearClippingPlane;
    return FMatrix(FPlane(XS, 0.0f, 0.0f, 0.0f), FPlane(0.0f, YS, 0.0f, 0.0f), FPlane(0.0f, 0.0f, 0.0f, 1.0f),
        FPlane(0.0f, 0.0f, InNearZ, 0.0f)) * FTranslationMatrix(FVector(PassProjectionOffset, 0, 0));
#endif
}

//获取眼睛渲染参数，被渲染线程调用
void FHuaweiVRSDK::GetEyeRenderParams_RenderThread(const FRenderingCompositePassContext& Context,
    FVector2D& EyeToSrcUVScaleValue, FVector2D& EyeToSrcUVOffsetValue) const
{
    LOGI("HvrLog: FHuaweiVRSDK::GetEyeRenderParams_RenderThread");
    EyeToSrcUVOffsetValue = FVector2D::ZeroVector;
    EyeToSrcUVScaleValue = FVector2D(1.0f, 1.0f);
}

//渲染纹理，被渲染线程调用
void FHuaweiVRSDK::RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D * BackBuffer,
    FRHITexture2D * SrcTexture, FVector2D WindowSize) const
{
    LOGI("HvrLog: FHuaweiVRSDK::RenderTexture_RenderThread");
    check(IsInRenderingThread());
#if PLATFORM_ANDROID
	if (CustomPresent->mIsMobileMultiViewState) {//如果是中心区渲染，则不在这个函数里面渲染，直接返回
		LOGI("FHuaweiVRSDK::RenderTexture_RenderThread mIsMobileMultiViewState");
		return;
	}
#endif

    const uint32 ViewportWidth = BackBuffer->GetSizeX();
    const uint32 ViewportHeight = BackBuffer->GetSizeY();
    const uint32 srcTextureWidth = SrcTexture->GetSizeX();
    const uint32 srcTextureHeight = SrcTexture->GetSizeY();

    // LOGI("RenderTexture_RenderThread() Viewport:(%d, %d) Texture:(%d, %d) BackBuffer=%p SrcTexture=%p",
    // ViewportWidth, ViewportHeight, srcTextureWidth, srcTextureHeight, BackBuffer, SrcTexture);

    //如果有定制的，且有特定渲染目标，就单独走定制流程
    if (CustomPresent && CustomPresent->m_RenderTargets) {
        LOGI("HvrLog: FHuaweiVRSDK::CustomPresent exist");
        SHWRenderTarget& RenderTarget = CustomPresent->m_RenderTargets->GetCurrentTarget();
        //初始化两只眼睛的渲染管线
        for (int nEye = 0; nEye < HW_EYE_COUNT; nEye++) {
            // RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
            // RHICmdList.SetRasterizerState(TStaticRasterizerState<>::GetRHI());
            // RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false, CF_Always>::GetRHI());
            FGraphicsPipelineStateInitializer GraphicsPSOInit;
            GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();//混合
            GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();//光栅化器
            GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();//深度模板
            // GraphicsPSOInit.NumSamples = 4;

            FRHITexture2D * eyeTexture = RenderTarget.EyeTexture[nEye];

            int nDstTextureWidth = eyeTexture->GetSizeX();
            int nDstTextureHeight = eyeTexture->GetSizeY();

            // LOGI("RenderTexture_RenderThread eyeTexture nDstTextureWidth = %d, nDstTextureHeight = %d",
            // nDstTextureWidth, nDstTextureHeight);

            // SetRenderTarget(RHICmdList, eyeTexture, FTextureRHIRef()); // 422
            FRHIRenderPassInfo RenderPassInfo(eyeTexture->GetTexture2D(), ERenderTargetActions::DontLoad_Store);

            //我觉得RHICmdList是一个发送指令的接口，每个函数代表着一个指令。
            RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("HuaweiVRSDK_RenderTexture"));//设置命令，开始渲染管线
            {
                RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);//设置命令：向缓存申请渲染目标的内存

                RHICmdList.SetViewport(0, 0, 0, nDstTextureWidth, nDstTextureHeight, 1.0f);////设置命令：设置视口

                const auto FeatureLevel = GMaxRHIFeatureLevel;
                auto ShaderMap = GetGlobalShaderMap(FeatureLevel);//获取全局着色器Map

                TShaderMapRef<FScreenVS> VertexShader(ShaderMap);//顶点着色器
                TShaderMapRef<FScreenPS> PixelShader(ShaderMap);//像素着色器

                // static FGlobalBoundShaderState BoundShaderState;
                // SetGlobalBoundShaderState(RHICmdList, FeatureLevel, BoundShaderState,
                // RendererModule->GetFilterVertexDeclaration().VertexDeclarationRHI, *VertexShader, *PixelShader);
                GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
                // GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader); // 422
                GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
                // GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader); // 422
                GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
                GraphicsPSOInit.PrimitiveType = PT_TriangleList;

                SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);//设置图形管线的状态，好像是注入命令和图形管线状态的init参数

                //往像素着色器里面设置参数，像素着色器不止一个
                PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Bilinear>::GetRHI(), SrcTexture);

                //这是绘制了一个矩形？
                RendererModule->DrawRectangle(RHICmdList, 0, 0, srcTextureWidth, srcTextureHeight, nEye * 0.5f, 0.0f, 1.0f,
                    1.0f, FIntPoint(nDstTextureWidth, nDstTextureHeight), FIntPoint(1, 1), VertexShader, EDRF_Default);
            }
            RHICmdList.EndRenderPass();//结束渲染通道
        }

        // UE_LOG(LogHuaweiVRSDK, Log, TEXT("TimeWarp RenderTexture_RenderThread  tw_idx=%d threadID=%d"),
        // RenderTarget.SensorState.twIdx, gettid());
        return;
    }

    //没有定制渲染（应该不会走到这里的）
    // RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
    // RHICmdList.SetRasterizerState(TStaticRasterizerState<>::GetRHI());
    // RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false, CF_Always>::GetRHI());
    LOGI("HvrLog: FHuaweiVRSDK::CustomPresent is not exist");
    FGraphicsPipelineStateInitializer GraphicsPSOInit;
    GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
    GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
    GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
    // GraphicsPSOInit.NumSamples = 4;
    // SetRenderTarget(RHICmdList, BackBuffer, FTextureRHIRef()); // 422
    FRHIRenderPassInfo RenderPassInfo(BackBuffer->GetTexture2D(), ERenderTargetActions::DontLoad_Store);
    RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("HuaweiVRSDK_RenderTexture"));
    {
        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

        RHICmdList.SetViewport(0, 0, 0, ViewportWidth, ViewportHeight, 1.0f);

        const auto FeatureLevel = GMaxRHIFeatureLevel;
        auto ShaderMap = GetGlobalShaderMap(FeatureLevel);

        TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
        TShaderMapRef<FScreenPS> PixelShader(ShaderMap);

        // static FGlobalBoundShaderState BoundShaderState;
        // SetGlobalBoundShaderState(RHICmdList, FeatureLevel, BoundShaderState,
        // RendererModule->GetFilterVertexDeclaration().VertexDeclarationRHI, *VertexShader, *PixelShader);
        GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
        // GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader); // 422 
        GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
        // GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);    // 422
        GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
        GraphicsPSOInit.PrimitiveType = PT_TriangleList;

        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Bilinear>::GetRHI(), SrcTexture);


        RendererModule->DrawRectangle(RHICmdList, 0, 0, srcTextureWidth, srcTextureHeight, 0.0f, 0.0f, 1.0f, 1.0f,
            FIntPoint(ViewportWidth, ViewportHeight), FIntPoint(1, 1), VertexShader, EDRF_Default);
    }
    RHICmdList.EndRenderPass();
}

//获取定制（自定义）显示
FHuaweiVRCustomPresent* FHuaweiVRSDK::GetCustomPresent()
{
    LOGI("HvrLog: FHuaweiVRSDK::GetCustomPresent");
    return CustomPresent;
}

//获取渲染目标管理者
IStereoRenderTargetManager* FHuaweiVRSDK::GetRenderTargetManager()
{
    LOGI("HvrLog: FHuaweiVRSDK::GetRenderTargetManager");
    return this;
}

//设置视口族 override
void FHuaweiVRSDK::SetupViewFamily(FSceneViewFamily& InViewFamily)
{
    LOGI("HvrLog: FHuaweiVRSDK::SetupViewFamily");
	InViewFamily.EngineShowFlags.MotionBlur = 0;
    InViewFamily.EngineShowFlags.HMDDistortion = false;
    InViewFamily.EngineShowFlags.SetScreenPercentage(true);
    InViewFamily.EngineShowFlags.ScreenPercentage = true;
    InViewFamily.EngineShowFlags.StereoRendering = IsStereoEnabled();
}

//设置视口（暂无用）override
void FHuaweiVRSDK::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
    LOGI("HvrLog: FHuaweiVRSDK::SetupView");
	/*
	InView.BaseHmdOrientation = FQuat(0.0f, 0.0f, 0.0f, 1.0f);
    InView.BaseHmdLocation = FVector(0.f);
	 */
}

//useless
void FHuaweiVRSDK::PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
    LOGI("HvrLog: FHuaweiVRSDK::PreRenderView_RenderThread, IsInRenderingThread() = %d", IsInRenderingThread());
    check(IsInRenderingThread());
}

//useless
void FHuaweiVRSDK::PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily)
{
    LOGI("HvrLog: FHuaweiVRSDK::PreRenderViewFamily_RenderThread, IsInRenderingThread() = %d", IsInRenderingThread());
    check(IsInRenderingThread());
}

//useless
void FHuaweiVRSDK::PostRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList,
    FSceneViewFamily& InViewFamily)
{
    LOGI("HvrLog: FHuaweiVRSDK::PostRenderViewFamily_RenderThread");
}

bool FHuaweiVRSDK::IsActiveThisFrame(class FViewport* InViewport) const
{
    LOGI("HvrLog: FHuaweiVRSDK::IsActiveThisFrame");
    return GEngine && GEngine->IsStereoscopic3D(InViewport);//用引擎判断是否应该活跃该帧
}

//是否应该使用立体渲染去渲染目标
bool FHuaweiVRSDK::ShouldUseSeparateRenderTarget() const
{
    LOGI("HvrLog: FHuaweiVRSDK::ShouldUseSeparateRenderTarget");
    return true;
}
/*
void FHuaweiVRSDK::CalculateRenderTargetSize(const class FViewport& Viewport, uint32& InOutSizeX, uint32& InOutSizeY) {
    //LOGI("FHuaweiVRSDK::CalculateRenderTargetSize %u %u, %u %u", Viewport.GetSizeXY().X, Viewport.GetSizeXY().Y,
InOutSizeX, InOutSizeY); InOutSizeX = RenderSizeX * 2; InOutSizeY = RenderSizeY;
}

bool FHuaweiVRSDK::NeedReAllocateViewportRenderTarget(const class FViewport& Viewport) {
    //UE_LOG(LogHuaweiVRSDK, Log, TEXT("FHuaweiVRSDK::NeedReAllocateViewportRenderTarget"));
    //LOGI("FHuaweiVRSDK::NeedReAllocateViewportRenderTarget");

    if (IsStereoEnabled()) {
        const uint32 InSizeX = Viewport.GetSizeXY().X;
        const uint32 InSizeY = Viewport.GetSizeXY().Y;
        const FIntPoint RenderTargetSize = Viewport.GetRenderTargetTextureSizeXY();

        uint32 NewSizeX = InSizeX, NewSizeY = InSizeY;
        CalculateRenderTargetSize(Viewport, NewSizeX, NewSizeY);
        //UE_LOG(LogHuaweiVRSDK, Log, TEXT("NeedReAllocateViewportRenderTarget() RenderTargetSize:(%d, %d) NewSize:(%d,
%d)"), RenderTargetSize.X, RenderTargetSize.Y, NewSizeX, NewSizeY);
        //LOGI("NeedReAllocateViewportRenderTarget() RenderTargetSize:(%u, %u) NewSize:(%u, %u), InSize:(%u, %u)",
RenderTargetSize.X, RenderTargetSize.Y, NewSizeX, NewSizeY, InSizeX, InSizeY);

        if (NewSizeX != RenderTargetSize.X || NewSizeY != RenderTargetSize.Y) {
            return true;
        }
    }
    return false;
}
*/
//分配渲染目标的纹理，返回值表示是否分配成功
bool FHuaweiVRSDK::AllocateRenderTargetTexture(uint32 Index, uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips,
    ETextureCreateFlags Flags, ETextureCreateFlags TargetableTextureFlags, FTexture2DRHIRef& OutTargetableTexture,
    FTexture2DRHIRef& OutShaderResourceTexture, uint32 NumSamples)
{
    LOGI("HvrLog: FHuaweiVRSDK::AllocateRenderTargetTexture, index = %u, SizeX, SizeY:(%u, %u), NumMips = %u, IsInGameThread = %d, IsInRenderingThread = %d, IsMultiviewState = %d ", Index, SizeX, SizeY, NumMips, IsInGameThread(), IsInRenderingThread(), IsMultiviewState);
    check(Index == 0);
	check(SizeX != 0 && SizeY != 0);
    // checking if rendering thread is suspended  检查渲染线程是否挂起
	check(IsInGameThread() && IsInRenderingThread()); 
    if (CustomPresent) {
        return CustomPresent->AllocateRenderTargetTexture(SizeX, SizeY, Format, NumMips, Flags, TargetableTextureFlags,
            OutTargetableTexture, OutShaderResourceTexture, NumSamples);
    }
    return false;
}

/* void FHuaweiVRSDK::UpdateViewportRHIBridge(bool bUseSeparateRenderTarget, const class FViewport& Viewport,
FRHIViewport* const ViewportRHI) {
    //UE_LOG(LogHuaweiVRSDK, Log, TEXT("FHuaweiVRSDK::UpdateViewportRHIBridge %d tid %d"), bUseSeparateRenderTarget,
gettid()); #if PLATFORM_ANDROID
    //LOGI("FHuaweiVRSDK::UpdateViewportRHIBridge %d tid %d", bUseSeparateRenderTarget, gettid());
#endif
    if (CustomPresent) {
        CustomPresent->UpdateViewport(ViewportRHI);
    }
} */

//game线程使用的，获取活跃的渲染桥梁
FXRRenderBridge* FHuaweiVRSDK::GetActiveRenderBridge_GameThread(bool bUseSeparateRenderTarget)
{
    LOGI("HvrLog: FHuaweiVRSDK::GetActiveRenderBridge_GameThread");
    // LOGI("FHuaweiVRSDK::GetActiveRenderBridge_GameThread bUseSeparateRenderTarget = %d", bUseSeparateRenderTarget);
    if (bUseSeparateRenderTarget) {
        return CustomPresent;
    } else {
        return nullptr;
    }
}

//应用暂停 委托
void FHuaweiVRSDK::ApplicationPauseDelegate()
{
    LOGI("HvrLog: FHuaweiVRSDK::ApplicationPauseDelegate");
    LOGI("FHuaweiVRSDK::ApplicationPauseDelegate");
}

//应用继续 委托
void FHuaweiVRSDK::ApplicationResumeDelegate()
{
    LOGI("HvrLog: FHuaweiVRSDK::ApplicationResumeDelegate");

#if PLATFORM_ANDROID
    int msgIdx = -1;
    // Clear messages when app resume
    HVR_GetMsgIdx(msgIdx);
#endif
}

FHuaweiVRSDK::FHuaweiVRSDK(const FAutoRegister& AutoRegister)
    : FHeadMountedDisplayBase(nullptr),
      FSceneViewExtensionBase(AutoRegister),
        setOffSet(false),
        lastRotation(FQuat::Identity),
        lastLocation(FVector::ZeroVector),
        teleport(false),
        CurHmdOrientation(FQuat::Identity),
        CurHmdPosition(FVector::ZeroVector),
        BaseOrientation(FQuat::Identity),
        BasePos(FVector::ZeroVector),
        DeltaControlRotation(FRotator::ZeroRotator),
        DeltaControlOrientation(FQuat::Identity),
        LastSensorTime(-1.0)
{
    LOGI("HvrLog: FHuaweiVRSDK::FHuaweiVRSDK");
    // Get renderer module
    static const FName RendererModuleName("Renderer");
    RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
    check(RendererModule);

    IConsoleVariable* CVarMCSF = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MobileContentScaleFactor"));
    CVarMCSF->Set(0.0f);

    IConsoleVariable* CVarMSAA = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MobileMSAA"));
    int32 msaa = CVarMSAA->GetInt();
#if PLATFORM_ANDROID
    LOGI("Current MSAA is %d", msaa);
    if (msaa < 2) {
        LOGI("Set default MSAA as 2");
        CVarMSAA->Set(2);
    }
#endif

    UpdateFrameRate();

    HuaweiVRSDKMessageTable::init();//没做操作
#if PLATFORM_ANDROID
    LOGI("%s COMPILE TIME : " __DATE__ " " __TIME__, TCHAR_TO_ANSI(*HuaweiVRSDKUnrealVersion));
    HVR_SendMessage("pluginVersion", TCHAR_TO_ANSI(*HuaweiVRSDKUnrealVersion));
#endif
    CustomPresent = new FHuaweiVRCustomPresent();//创建自定义显示器

#if PLATFORM_ANDROID
	IsMultiviewState = CustomPresent->mIsMobileMultiViewState;
#endif

    FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddRaw(this, &FHuaweiVRSDK::ApplicationPauseDelegate);
    FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddRaw(this, &FHuaweiVRSDK::ApplicationResumeDelegate);

}

FString FHuaweiVRSDK::GetHuaweiVRSDKVersion()
{
    LOGI("HvrLog: FHuaweiVRSDK::GetHuaweiVRSDKVersion");
    return HuaweiVRSDKUnrealVersion;
}

int FHuaweiVRSDK::GetHMDInfo(HelmetModel& helmetModel)
{
    LOGI("HvrLog: FHuaweiVRSDK::GetHMDInfo");
#if PLATFORM_ANDROID
    return HVR_Android_GetHelmetInfo(HVR_Android_GetHelmet(), &helmetModel);
#else
    return -1;
#endif
}

FHuaweiVRSDK::~FHuaweiVRSDK()
{
    LOGI("HvrLog: FHuaweiVRSDK::~FHuaweiVRSDK");
    if (nullptr != CustomPresent) {
        delete CustomPresent;
        CustomPresent = nullptr;
    }
}

void FHuaweiVRSDK::SetRenderSize(int width, int height)
{
	if (RenderSizeX > width) {
		return;
	}
    LOGI("HvrLog: FHuaweiVRSDK::SetRenderSize");
	if (IsMultiviewState) {
		RenderSizeX = HUAWEI_RT_WIDTH_MULTIVIEW_H;//如果是多视口，则渲染款宽高设为固定值（1280*1280）
        RenderSizeY = HUAWEI_RT_WIDTH_MULTIVIEW_H;
	} else {
		RenderSizeX = width;
		RenderSizeY = height;
	}

}

//设置渲染帧率
void FHuaweiVRSDK::SetRenderFrameRate(int frameRate)
{
    LOGI("HvrLog: FHuaweiVRSDK::SetRenderFrameRate %d, current RenderFrameRate %d", frameRate, RenderFrameRate);
    if (RenderFrameRate != frameRate) {
        RenderFPSChanged = true;
    }
    RenderFrameRate = frameRate;
}

//更新渲染帧率，应用帧率
void FHuaweiVRSDK::UpdateFrameRate()
{
    LOGI("HvrLog: FHuaweiVRSDK::UpdateFrameRate");
    check(IsInGameThread());
    IConsoleVariable* MaxFPSCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"));
    if (NULL == MaxFPSCVar) {
        LOGE("FHuaweiVRSDK::UpdateFrameRate Cannot find CVar t.MaxFPS");
        return;
    }
    int currentFPS = MaxFPSCVar->GetInt();
	LOGI("FHuaweiVRSDK::UpdateFrameRate currentMaxFPS %d, RenderFrameRate %d", currentFPS, RenderFrameRate);
    if (currentFPS != RenderFrameRate) {
        LOGI("FHuaweiVRSDK::UpdateFrameRate RenderFrameRate %d", RenderFrameRate);
        MaxFPSCVar->Set(RenderFrameRate);
    }
    RenderFPSChanged = false;
}

//返回是否初始化
bool FHuaweiVRSDK::IsInitialized() const
{
    LOGI("HvrLog: FHuaweiVRSDK::IsInitialized");
    return true;
}

//更新视口size
void FHuaweiVRSDK::UpdateViewportSize()
{
    LOGI("HvrLog: FHuaweiVRSDK::UpdateViewportSize");
    check(IsInGameThread());
    if (GWorld) {
        UGameViewportClient* gvc = GWorld->GetGameViewport();
        if (gvc) {
            FVector2D viewportSize;
            gvc->GetViewportSize(viewportSize);
            FSceneViewport* svp = gvc->GetGameViewport();
            if (svp) {
                FIntPoint fip = svp->GetSize();
                if (fip.X != (RenderSizeX * 2) || fip.Y != RenderSizeY) {
                    LOGI("UpdateViewportSize SetSize (%d, %d)", RenderSizeX, RenderSizeY);
                    svp->SetViewportSize(RenderSizeX * 2, RenderSizeY);
                }
            }
        }
    }
}


//获取VR信息，更新帧率，更新视口size
bool FHuaweiVRSDK::GetHuaweiVRMessage(int& type, int& priority, int& boxType, TMap<FString, FString>& message)
{
    LOGI("HvrLog: FHuaweiVRSDK::GetHuaweiVRMessage");
    bool ret = HuaweiVRSDKMessageTable::QueryMessage(type, priority, boxType, message);

    if (RenderFPSChanged) {
        UpdateFrameRate();
    }

    UpdateViewportSize();

    return ret;
}

int FHuaweiVRSDK::ResetYaw()
{
#if PLATFORM_ANDROID
    return HVR_ResetSensorYaw();
#else
    return 0;
#endif
}

int FHuaweiVRSDK::SetPoseLock(bool enable)
{
#if PLATFORM_ANDROID
    return HVR_Android_SetPoseLock(HVR_Android_GetHelmet(), enable);
#else
    return 0;
#endif
}

//设置色像差
int FHuaweiVRSDK::SetChromaticAberration(bool enable)
{
#if PLATFORM_ANDROID
    HVR_Android_EnableChromaticAberration(enable);
    return 0;
#else
    return 0;
#endif
}


int FHuaweiVRSDK::SetSvsEffect(bool enable)
{
#if PLATFORM_ANDROID
    return HVR_Android_EnableSvsEffect(enable);
#else
    return 0;
#endif
}