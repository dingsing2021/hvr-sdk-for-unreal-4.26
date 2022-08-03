// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RendererPrivate.h"
#include "XRRenderBridge.h"
#include "HAL/ThreadSafeBool.h"

#if !WITH_EDITOR
#include "OpenGLDrvPrivate.h"
#include "OpenGLResources.h"
#endif

//#include "OpenGLDrv/Public/OpenGLResources.h"

#define HW_RENDERTARGET_COUNT 6
#define HW_EYE_COUNT 2
#define HW_RENDERTARGETTXCTURENUM 6

//传感器状态（位姿）
struct SensorState {
    int twIdx;
    float w, x, y, z, fov;//姿态的四个分量 以及fov
    float px, py, pz;//位置的三个分量
    SensorState()
    {
        Reset();
    }
    void Reset()
    {
        w = 1.0f;
        x = y = z = 0.0f;
        px = py = pz = 0.0f;
        fov = 95.0f;
        twIdx = 0;
    }
};


struct SHWRenderTarget {
    FTexture2DRHIRef EyeTexture[HW_EYE_COUNT];
    SensorState SensorState;
};

//OpenGL纹理
// for Foveate
#if PLATFORM_ANDROID
class FHWMobileHMDTextureSet :public FOpenGLTexture2D
{
public:
	static FHWMobileHMDTextureSet* CreateTexture2DSet(
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
		FTexture2DRHIRef& OutShaderResourceTexture);

	FHWMobileHMDTextureSet(
		class FOpenGLDynamicRHI* InGLRHI,
		//GLuint InResource,
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
		ETextureCreateFlags InTargetableTextureFlags);

	virtual ~FHWMobileHMDTextureSet() override;

	void SwitchToNextElement();
	void SetIndexAsCurrentRenderTarget();
	GLuint GetRenderTargetResource();
	GLuint GetBlackTextureResource();
	GLuint GetReticleTextureResource();

	//FTexture2DRHIRef    mRenderTargetTextureRHIRef[HW_RENDERTARGETTXCTURENUM];
	FTexture2DRHIRef    mRenderTargetTextureRHIRefTy;
	//FTexture2DRHIRef
	FTexture2DRHIRef	TextureRef;

private:
	FHWMobileHMDTextureSet(const FHWMobileHMDTextureSet &) = delete;
	FHWMobileHMDTextureSet(FHWMobileHMDTextureSet &&) = delete;
	FHWMobileHMDTextureSet &operator=(const FHWMobileHMDTextureSet &) = delete;


	//FTextureRHIRef    mRenderTargetTextureRHIRef[HW_RENDERTARGETTXCTURENUM];
	FTextureRHIRef		mRenderTargetTextureRHIRef[HW_RENDERTARGETTXCTURENUM];
	int                 mRenderTargetTextureRHIRefIndex;

	FTexture2DRHIRef ReticleTexture;

	FTextureRHIRef BlackTexture;
};
#endif

//渲染目标
class FHWRenderTargets {
    friend class FHuaweiVRCustomPresent;
    friend class FHuaweiVRSDK;

public:
    FHWRenderTargets();
    ~FHWRenderTargets();

    void ReleaseResources() {}

    void SwitchToNextElement();

    void InitRenderTarget(uint32 InSizeX, uint32 InSizeY);

    SHWRenderTarget& GetCurrentTarget();
    SHWRenderTarget& GetLastTarget();

protected:
    uint32 CurrentIndex;
    FTexture2DRHIRef TextureRef;
    SHWRenderTarget Target[HW_RENDERTARGET_COUNT];
};
/* *
 *
 */
//显示单位只有一个
class FHuaweiVRCustomPresent : public FXRRenderBridge {
    friend class FHuaweiVRSDK;

public:
    FHuaweiVRCustomPresent();
    ~FHuaweiVRCustomPresent();

    // Called when viewport is resized.当视口size被修改时被调用
    virtual void OnBackBufferResize() override;//固定不做操作，只是打印一行日志

    //被渲染线程调用以查看这个帧是否需要一个本地显示去显示它，nativePresent值的就是这个自定义显示器。固定返回真
    // Called from render thread to see if a native present will be requested for this frame.
    // @return    true if native Present will be requested for this frame; false otherwise.  Must
    // match value subsequently returned by Present for this frame.
    virtual bool NeedsNativePresent() override;

    //被RHI线程调用以执行自定义显示器（被RHI线程调用以显示一帧）
    // Called from RHI thread to perform custom present.
    // @param InOutSyncInterval - in out param, indicates if vsync is on (>0) or off (==0).
    // @return    true if native Present should be also be performed; false otherwise. If it returns
    // true, then InOutSyncInterval could be modified to switch between VSync/NoVSync for the normal
    // Present.  Must match value previously returned by NeedsNormalPresent for this frame.
    virtual bool Present(int32& InOutSyncInterval) override;

    bool AllocateRenderTargetTexture(uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, ETextureCreateFlags Flags,
        ETextureCreateFlags TargetableTextureFlags, FTexture2DRHIRef& OutTargetableTexture,
        FTexture2DRHIRef& OutShaderResourceTexture, uint32 NumSamples);

    void Init();
    void Shutdown();

    // Frame operations
    void BeginRendering(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily);
    void FinishRendering();
    // void UpdateViewport(const class FViewport& Viewport, class FRHIViewport* InViewportRHI) override;

    // Called when rendering thread is acquired 当获取渲染线程时调用这个函数
    virtual void OnAcquireThreadOwnership() override;
    // Called when rendering thread is released 当释放渲染线程时调用这个函数
    virtual void OnReleaseThreadOwnership() override; // 422

    void MainThreadRunningNotify(bool running);

    void ConditionalUpdateCache();

public:
    FHWRenderTargets* m_RenderTargets;//里面包含了6个渲染目标

	int  mEyeType;
    bool allowUpdatePose = true;

#if PLATFORM_ANDROID
	TRefCountPtr<FHWMobileHMDTextureSet> mTextureSet;//纹理的集合，因为是把画面渲染到这些纹理上的，之所以有多个纹理是因为中心区渲染
	uint mGlTexIDTextureRefLeft;
	uint mGlTexIDTextureRefRight;
	bool mIsMobileMultiViewState;
#endif

private:
     
#if PLATFORM_ANDROID
	FThreadSafeBool mMainThreadRunning;
	FThreadSafeBool mIsThreadOwnershipAcquired;//是否已经取得线程所有权
	bool mOnApplicationStart;
    bool mOnresume;
#endif
};