// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HeadMountedDisplayBase.h"
#include "XRTrackingSystemBase.h"
#include "SceneViewExtension.h"
#include "XRRenderTargetManager.h"
#include "HAL/ThreadSafeBool.h"
#include "GameFramework/Actor.h"

#include "HuaweiVRSDKAPI.h"
#include "HuaweiVRCustomPresent.h"

DEFINE_LOG_CATEGORY_STATIC(LogHuaweiVRSDK, Log, All);

class FSceneView;
class FSceneViewFamily;


/* *
 * Simple Head Mounted Display
 */
class FHuaweiVRSDK : public FHeadMountedDisplayBase, public FSceneViewExtensionBase, public FXRRenderTargetManager {
    friend class FHuaweiVRCustomPresent;

public:
    static FHuaweiVRSDK* GetHuaweiVRSDK();

    /* * IXRTrackingSystem interface */
    //为什么要有一个F，是根据名字来把我们的类关联起来的，这是Unreal定义的一些接口，我们要去实现它
    virtual FName GetSystemName() const override
    {
        static FName DefaultName(TEXT("HuaweiVRSDK"));
        return DefaultName;
    }

    virtual bool EnumerateTrackedDevices(TArray<int32>& OutDevices,
        EXRTrackedDeviceType Type = EXRTrackedDeviceType::Any) override;

    virtual void SetInterpupillaryDistance(float NewInterpupillaryDistance) override;
    virtual float GetInterpupillaryDistance() const override;

    virtual void ResetOrientationAndPosition(float yaw = 0.f) override;
    virtual void ResetOrientation(float Yaw = 0.f) override;
    virtual void ResetPosition() override;
	virtual bool GetRelativeEyePose(int32 DeviceId, EStereoscopicPass StereoPassType, FQuat& OutOrientation, FVector& OutPosition) override;
    virtual bool GetCurrentPose(int32 DeviceId, FQuat& CurrentOrientation, FVector& CurrentPosition) override;
    virtual void SetBaseRotation(const FRotator& BaseRot) override;
    virtual FRotator GetBaseRotation() const override;

    virtual void SetBaseOrientation(const FQuat& BaseOrient) override;
    virtual FQuat GetBaseOrientation() const override;

    virtual void SetBasePosition(const FVector& BasePosition) override;
    virtual FVector GetBasePosition() const override;

    virtual bool OnEndGameFrame(FWorldContext& WorldContext) override;
    virtual class IHeadMountedDisplay* GetHMDDevice() override
    {
        return this;
    }

    virtual class TSharedPtr<class IStereoRendering, ESPMode::ThreadSafe> GetStereoRenderingDevice() override {
        return SharedThis(this);
    }

        /* * HuaweiVRSDK API */
        FHuaweiVRCustomPresent*
        GetCustomPresent();

    FString GetHuaweiVRSDKVersion();

    int GetHMDInfo(HelmetModel& helmetModel);

    void GetHMDOrientationAndPosition(FQuat& CurrentOrientation, FVector& CurrentPosition);
    int ResetYaw();
    int SetPoseLock(bool enable);
    int SetChromaticAberration(bool enable);
    int SetSvsEffect(bool enable);

    /**
     * Returns device specific flags.
     */
    virtual int32 GetXRSystemFlags() const { return 0; };

protected:
    /* * FXRTrackingSystemBase protected interface */
    virtual float GetWorldToMetersScale() const override;

public:
    /* * IHeadMountedDisplay interface */
    virtual bool IsHMDConnected() override
    {
        return true;
    }
    virtual bool IsHMDEnabled() const override;
    virtual void EnableHMD(bool allow = true) override;
    virtual bool GetHMDMonitorInfo(MonitorInfo&) override;
    virtual void GetFieldOfView(float& OutHFOVInDegrees, float& OutVFOVInDegrees) const override;
    virtual bool IsChromaAbCorrectionEnabled() const override;
    virtual void DrawDistortionMesh_RenderThread(struct FRenderingCompositePassContext& Context,
        const FIntPoint& TextureSize) override;
    virtual FIntPoint GetIdealRenderTargetSize() const override;

    /* * IStereoRendering interface */
    virtual bool IsStereoEnabled() const override;
    virtual bool EnableStereo(bool stereo = true) override;
    virtual void AdjustViewRect(EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX,
        uint32& SizeY) const override;
    virtual void CalculateStereoViewOffset(const EStereoscopicPass StereoPassType, FRotator& ViewRotation,
        const float MetersToWorld, FVector& ViewLocation) override;
    virtual FMatrix GetStereoProjectionMatrix(const enum EStereoscopicPass StereoPassType) const override;
    virtual void GetEyeRenderParams_RenderThread(const struct FRenderingCompositePassContext& Context,
        FVector2D& EyeToSrcUVScaleValue, FVector2D& EyeToSrcUVOffsetValue) const override;
    virtual void RenderTexture_RenderThread(class FRHICommandListImmediate& RHICmdList, class FRHITexture2D* BackBuffer,
        class FRHITexture2D* SrcTexture, FVector2D WindowSize) const override;
    virtual IStereoRenderTargetManager* GetRenderTargetManager() override;

    /* * ISceneViewExtension interface */
    virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override;
    virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;
    virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) {}
    virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override;
    virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList,
        FSceneViewFamily& InViewFamily) override;
    virtual void PostRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList,
        FSceneViewFamily& InViewFamily) override;
    virtual bool IsActiveThisFrame(class FViewport* InViewport) const;

    /* * IStereoRenderTargetManager interface */
    virtual bool ShouldUseSeparateRenderTarget() const override;//是否应该使用立体渲染去渲染目标
    // virtual void CalculateRenderTargetSize(const class FViewport& Viewport, uint32& InOutSizeX, uint32& InOutSizeY)
    // override; virtual bool NeedReAllocateViewportRenderTarget(const class FViewport& Viewport) override;
    virtual bool AllocateRenderTargetTexture(uint32 Index, uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips,
        ETextureCreateFlags Flags, ETextureCreateFlags TargetableTextureFlags, FTexture2DRHIRef& OutTargetableTexture,
        FTexture2DRHIRef& OutShaderResourceTexture, uint32 NumSamples = 1) override;
    // virtual void UpdateViewportRHIBridge(bool bUseSeparateRenderTarget, const class FViewport& Viewport,
    // FRHIViewport* const ViewportRHI) override;
    virtual FXRRenderBridge* GetActiveRenderBridge_GameThread(bool bUseSeparateRenderTarget) override;

    void ApplicationPauseDelegate();//app暂停 委托
    void ApplicationResumeDelegate();//app继续 委托

    bool GetHuaweiVRMessage(int& type, int& priority, int& boxType, TMap<FString, FString>& message);//获取VR相关数据
	static void SetRenderSize(int width, int height);//设置渲染尺寸
    static void SetRenderFrameRate(int frameRate);//设置帧率

public:
    /* * Constructor */
    FHuaweiVRSDK(const FAutoRegister&);

    /* * Destructor */
    virtual ~FHuaweiVRSDK();

    /* * @return    True if the HMD was initialized OK */
    bool IsInitialized() const;

private:
    void UpdateFrameRate();
    void UpdateViewportSize();

private:
    static FThreadSafeBool RenderFPSChanged;
    static int RenderFrameRate;
    static int RenderSizeX;
    static int RenderSizeY;
	static bool IsMultiviewState;
	bool setOffSet;
	FRotator lastRotation;
	FVector lastLocation;
	bool teleport;

    FQuat CurHmdOrientation;
    FVector CurHmdPosition;

	FRotator HMDOffsetOrientation;
	FVector HMDOffsetPosition;

    FQuat BaseOrientation;
    FVector BasePos;
	FVector BaseViewPositionTemp;// for set base view location, while base rotation is set by playerController

    FRotator DeltaControlRotation; // same as DeltaControlOrientation but as rotator
    FQuat DeltaControlOrientation; // same as DeltaControlRotation but as quat

    double LastSensorTime;
	bool GetFirstTrans;
    bool mGameFrameEnded;

    IRendererModule* RendererModule;
    TRefCountPtr<FHuaweiVRCustomPresent> CustomPresent;
};
