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

IMPLEMENT_MODULE(FHuaweiVRSDKPlugin, HuaweiVRSDK)//IMPLEMENT_MODULE����꣬���������ǵĲ��������

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

//��ȡHMD��������Ϣ
bool FHuaweiVRSDK::GetHMDMonitorInfo(MonitorInfo& MonitorDesc)
{
    LOGI("HvrLog: FHuaweiVRSDK::GetHMDMonitorInfo");
    MonitorDesc.MonitorName = "";
    MonitorDesc.MonitorId = 0;
    MonitorDesc.DesktopX = MonitorDesc.DesktopY = MonitorDesc.ResolutionX = MonitorDesc.ResolutionY = 0;
    return false;
}

//��ȡ�ӳ���
void FHuaweiVRSDK::GetFieldOfView(float& OutHFOVInDegrees, float& OutVFOVInDegrees) const
{
    LOGI("HvrLog: FHuaweiVRSDK::GetFieldOfView");
    OutHFOVInDegrees = 0.0f;
    OutVFOVInDegrees = 0.0f;
}

//ö���豸
bool FHuaweiVRSDK::EnumerateTrackedDevices(TArray<int32>& OutDevices, EXRTrackedDeviceType Type)
{
    LOGI("HvrLog: FHuaweiVRSDK::EnumerateTrackedDevices");
    if (Type == EXRTrackedDeviceType::Any || Type == EXRTrackedDeviceType::HeadMountedDisplay) {
        OutDevices.Add(IXRTrackingSystem::HMDDeviceId);//��ʵ���Ǿ�һ���豸�����Խ�add�������
        return true;
    }
    return false;
}

void FHuaweiVRSDK::SetInterpupillaryDistance(float NewInterpupillaryDistance) { LOGI("HvrLog: FHuaweiVRSDK::SetInterpupillaryDistance"); }

//ȡͫ�׼��
float FHuaweiVRSDK::GetInterpupillaryDistance() const
{
    LOGI("HvrLog: FHuaweiVRSDK::GetInterpupillaryDistance");
    return 0.064f;
}

//ȡHMD�����λ��
void FHuaweiVRSDK::GetHMDOrientationAndPosition(FQuat& CurrentOrientation, FVector& CurrentPosition)
{
    LOGI("HvrLog: FHuaweiVRSDK::GetHMDOrientationAndPosition");
    // very basic.  no head model, no prediction, using debuglocalplayer
    // ULocalPlayer* Player = GEngine->GetDebugLocalPlayer();

    CurrentOrientation = CurHmdOrientation;
    CurrentPosition = CurHmdPosition;
	//if (CustomPresent && CustomPresent->m_RenderTargets) {
 //       //CustomPresent->ConditionalUpdateCache();//wxq:  ˢ�´���������

 //       //SHWRenderTarget& RenderTarget = CustomPresent->m_RenderTargets->GetCurrentTarget();
 //       //SensorState& st = RenderTarget.SensorState;

	//	//LOGI("CurrentPosition:%f %f %f", CurrentPosition.X, CurrentPosition.Y, CurrentPosition.Z);
 //   } else {
 //       LOGW("GetHMDOrientationAndPosition failed");
 //       CurrentOrientation = FQuat(0.0f, 0.0f, 0.0f, 1.0f);
 //       CurrentPosition = FVector(0.0f, 0.0f, 0.0f);
 //   }
}

//ȡend��Ϸ֡
bool FHuaweiVRSDK::OnEndGameFrame(FWorldContext& WorldContext)
{
    mGameFrameEnded = true;
    return false;
}

//���Ǻ����� ��ȡ��ǰ��̬�����û�и�д����ʹ��UE�Զ����,��̬���� λ��&����
//��Ⱦһ֡�����7���������
bool FHuaweiVRSDK::GetCurrentPose(int32 DeviceId, FQuat& CurrentOrientation, FVector& CurrentPosition)
{
    LOGI("HvrLog: FHuaweiVRSDK::GetCurrentPose");
    if (DeviceId != IXRTrackingSystem::HMDDeviceId) {//����豸ID���ԣ�����false
        LOGI("hvrlog: DeviceId != IXRTrackingSystem::HMDDeviceId ");
        return false;
    }
    
    if (CustomPresent && CustomPresent->m_RenderTargets) {
        if (CustomPresent->allowUpdatePose) {
            CustomPresent->ConditionalUpdateCache();//wxq:  ˢ�´���������
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

//�Ƿ�������ChromaAbУ��
bool FHuaweiVRSDK::IsChromaAbCorrectionEnabled() const
{
    LOGI("HvrLog: FHuaweiVRSDK::IsChromaAbCorrectionEnabled");
    return false;
}

//���÷����λ��
void FHuaweiVRSDK::ResetOrientationAndPosition(float yaw)
{
    LOGI("HvrLog: FHuaweiVRSDK::ResetOrientationAndPosition");
    ResetOrientation(yaw);
    ResetPosition();
}

//������ת��
void FHuaweiVRSDK::ResetOrientation(float Yaw)
{
    LOGI("HvrLog: FHuaweiVRSDK::ResetOrientation");
#if PLATFORM_ANDROID
    HVR_ResetSensorOrientation();//���ô���������ת��
#endif
}

//����λ�� useless
void FHuaweiVRSDK::ResetPosition() { LOGI("HvrLog: FHuaweiVRSDK::ResetPosition"); }

//���û�����ת��
void FHuaweiVRSDK::SetBaseRotation(const FRotator& BaseRot) {
    LOGI("HvrLog: FHuaweiVRSDK::SetBaseRotation");
}

//��ȡ������ת��
FRotator FHuaweiVRSDK::GetBaseRotation() const
{
    LOGI("HvrLog: FHuaweiVRSDK::GetBaseRotation");
    return FRotator::ZeroRotator;
}

//���û�������û�е��ã�
void FHuaweiVRSDK::SetBaseOrientation(const FQuat& BaseOrient) {
    LOGI("HvrLog: FHuaweiVRSDK::SetBaseOrientation");
	BaseOrientation = BaseOrient;
}

//��ȡ��������(û�е���)
FQuat FHuaweiVRSDK::GetBaseOrientation() const
{
    LOGI("HvrLog: FHuaweiVRSDK::GetBaseOrientation");
    return BaseOrientation;
}

//���û���λ��
void FHuaweiVRSDK::SetBasePosition(const FVector& BasePosition) {
    LOGI("HvrLog: FHuaweiVRSDK::SetBasePosition");
	BasePos = BasePosition;
	//LOGI("SetBasePosition BasePos:%f %f %f", BasePosition.X, BasePosition.Y, BasePosition.Z);
}

//��ȡ����λ��
FVector FHuaweiVRSDK::GetBasePosition() const
{
    LOGI("HvrLog: FHuaweiVRSDK::GetBasePosition");
    return BasePos;
}

//���Ʒ������� ����Ⱦ�̵߳���
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

//��ȡIdea������ģ���ȾĿ��ߴ�
FIntPoint FHuaweiVRSDK::GetIdealRenderTargetSize() const
{
     LOGI("HvrLog: GetIdealRenderTargetSize RenderSizeX = %d, RenderSizeY = %d", RenderSizeX, RenderSizeY);
    return FIntPoint(RenderSizeX * 2, RenderSizeY); // 1552
}

//�Ƿ����� ʹ��
bool FHuaweiVRSDK::IsStereoEnabled() const
{
    LOGI("HvrLog: FHuaweiVRSDK::IsStereoEnabled");
    return true;
}

//����ʹ��
bool FHuaweiVRSDK::EnableStereo(bool stereo)
{
    LOGI("HvrLog: FHuaweiVRSDK::EnableStereo");
    return true;
}

//�����ӿھ��Σ������þ���ζ�ţ���ͨ����Ⱦ���л��ӿڣ�
void FHuaweiVRSDK::AdjustViewRect(EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const
{ 
	SizeX = RenderSizeX * 2;
	SizeY = RenderSizeY;
#if PLATFORM_ANDROID
	if (!CustomPresent->mIsMobileMultiViewState) {
		SizeX = SizeX / 2;
	}
#endif 

    if (StereoPass == eSSP_RIGHT_EYE) {//�������Ⱦ���ۣ����һ��ͼ��ƫ��
        X += SizeX;
    }
    LOGI("HvrLog: FHuaweiVRSDK::AdjustViewRect (x, y):(%d, %d) (SizeX, SizeY):(%u, %u)", X, Y, SizeX, SizeY);
}

//����������ͼ��ƫ�ƣ��õ���̬�������UE���ã�
void FHuaweiVRSDK::CalculateStereoViewOffset(const enum EStereoscopicPass StereoPassType, FRotator& ViewRotation,
    const float WorldToMeters, FVector& ViewLocation)//����Ǽ̳���
{
    LOGI("HvrLog: FHuaweiVRSDK::, StereoPassType = %d", StereoPassType);
    //LOGI("HvrLog: FHuaweiVRSDK::StereoPassType1 = %d, Do[GetHMDOrientationAndPosition]", StereoPassType);
 //   if (StereoPassType == eSSP_LEFT_EYE) {
 //       GetHMDOrientationAndPosition(CurHmdOrientation, CurHmdPosition);//��ȡHMD �����λ�ã�����hmdλ��
	//}
    //if (StereoPassType == 0) {
    //    GetCurrentPose(HMDDeviceId, CurHmdOrientation, CurHmdPosition);//��ȡHMD �����λ�ã�����hmdλ��
    //}
	FHeadMountedDisplayBase::CalculateStereoViewOffset(StereoPassType, ViewRotation, WorldToMeters, ViewLocation);
	//LOGI("CalculateStereoViewOffset ViewLocation:%f %f %f", ViewLocation.X, ViewLocation.Y, ViewLocation.Z);
}

//��ȡ����۾�����
bool  FHuaweiVRSDK::GetRelativeEyePose(int32 DeviceId, EStereoscopicPass StereoPassType, FQuat& OutOrientation, FVector& OutPosition)
{
    LOGI("HvrLog: FHuaweiVRSDK::GetRelativeEyePose");
	OutOrientation = FQuat::Identity;//����̶�
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

//��ȡ����ͶӰ��������ͶӰ��,�����������ε���
FMatrix FHuaweiVRSDK::GetStereoProjectionMatrix(const enum EStereoscopicPass StereoPassType) const
{
    LOGI("HvrLog: FHuaweiVRSDK::GetStereoProjectionMatrix");
#if PLATFORM_ANDROID
    float fFOV = 95.0f;//fov�̶�95��
    HVR_GetFOVAngle(fFOV);//���²��ȡFOV���²��FOVҲ��д����95

    //ָ��ʾ��������������Ⱦ��eSSP_LEFT_EYEӦ���Ǳ�ʾ����ִ����������Ⱦ
	if (CustomPresent->mIsMobileMultiViewState && (StereoPassType == eSSP_LEFT_EYE)) {//����������
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

//��ȡ�۾���Ⱦ����������Ⱦ�̵߳���
void FHuaweiVRSDK::GetEyeRenderParams_RenderThread(const FRenderingCompositePassContext& Context,
    FVector2D& EyeToSrcUVScaleValue, FVector2D& EyeToSrcUVOffsetValue) const
{
    LOGI("HvrLog: FHuaweiVRSDK::GetEyeRenderParams_RenderThread");
    EyeToSrcUVOffsetValue = FVector2D::ZeroVector;
    EyeToSrcUVScaleValue = FVector2D(1.0f, 1.0f);
}

//��Ⱦ��������Ⱦ�̵߳���
void FHuaweiVRSDK::RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D * BackBuffer,
    FRHITexture2D * SrcTexture, FVector2D WindowSize) const
{
    LOGI("HvrLog: FHuaweiVRSDK::RenderTexture_RenderThread");
    check(IsInRenderingThread());
#if PLATFORM_ANDROID
	if (CustomPresent->mIsMobileMultiViewState) {//�������������Ⱦ�������������������Ⱦ��ֱ�ӷ���
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

    //����ж��Ƶģ������ض���ȾĿ�꣬�͵����߶�������
    if (CustomPresent && CustomPresent->m_RenderTargets) {
        LOGI("HvrLog: FHuaweiVRSDK::CustomPresent exist");
        SHWRenderTarget& RenderTarget = CustomPresent->m_RenderTargets->GetCurrentTarget();
        //��ʼ����ֻ�۾�����Ⱦ����
        for (int nEye = 0; nEye < HW_EYE_COUNT; nEye++) {
            // RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
            // RHICmdList.SetRasterizerState(TStaticRasterizerState<>::GetRHI());
            // RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false, CF_Always>::GetRHI());
            FGraphicsPipelineStateInitializer GraphicsPSOInit;
            GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();//���
            GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();//��դ����
            GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();//���ģ��
            // GraphicsPSOInit.NumSamples = 4;

            FRHITexture2D * eyeTexture = RenderTarget.EyeTexture[nEye];

            int nDstTextureWidth = eyeTexture->GetSizeX();
            int nDstTextureHeight = eyeTexture->GetSizeY();

            // LOGI("RenderTexture_RenderThread eyeTexture nDstTextureWidth = %d, nDstTextureHeight = %d",
            // nDstTextureWidth, nDstTextureHeight);

            // SetRenderTarget(RHICmdList, eyeTexture, FTextureRHIRef()); // 422
            FRHIRenderPassInfo RenderPassInfo(eyeTexture->GetTexture2D(), ERenderTargetActions::DontLoad_Store);

            //�Ҿ���RHICmdList��һ������ָ��Ľӿڣ�ÿ������������һ��ָ�
            RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("HuaweiVRSDK_RenderTexture"));//���������ʼ��Ⱦ����
            {
                RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);//��������򻺴�������ȾĿ����ڴ�

                RHICmdList.SetViewport(0, 0, 0, nDstTextureWidth, nDstTextureHeight, 1.0f);////������������ӿ�

                const auto FeatureLevel = GMaxRHIFeatureLevel;
                auto ShaderMap = GetGlobalShaderMap(FeatureLevel);//��ȡȫ����ɫ��Map

                TShaderMapRef<FScreenVS> VertexShader(ShaderMap);//������ɫ��
                TShaderMapRef<FScreenPS> PixelShader(ShaderMap);//������ɫ��

                // static FGlobalBoundShaderState BoundShaderState;
                // SetGlobalBoundShaderState(RHICmdList, FeatureLevel, BoundShaderState,
                // RendererModule->GetFilterVertexDeclaration().VertexDeclarationRHI, *VertexShader, *PixelShader);
                GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
                // GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader); // 422
                GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
                // GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader); // 422
                GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
                GraphicsPSOInit.PrimitiveType = PT_TriangleList;

                SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);//����ͼ�ι��ߵ�״̬��������ע�������ͼ�ι���״̬��init����

                //��������ɫ���������ò�����������ɫ����ֹһ��
                PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Bilinear>::GetRHI(), SrcTexture);

                //���ǻ�����һ�����Σ�
                RendererModule->DrawRectangle(RHICmdList, 0, 0, srcTextureWidth, srcTextureHeight, nEye * 0.5f, 0.0f, 1.0f,
                    1.0f, FIntPoint(nDstTextureWidth, nDstTextureHeight), FIntPoint(1, 1), VertexShader, EDRF_Default);
            }
            RHICmdList.EndRenderPass();//������Ⱦͨ��
        }

        // UE_LOG(LogHuaweiVRSDK, Log, TEXT("TimeWarp RenderTexture_RenderThread  tw_idx=%d threadID=%d"),
        // RenderTarget.SensorState.twIdx, gettid());
        return;
    }

    //û�ж�����Ⱦ��Ӧ�ò����ߵ�����ģ�
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

//��ȡ���ƣ��Զ��壩��ʾ
FHuaweiVRCustomPresent* FHuaweiVRSDK::GetCustomPresent()
{
    LOGI("HvrLog: FHuaweiVRSDK::GetCustomPresent");
    return CustomPresent;
}

//��ȡ��ȾĿ�������
IStereoRenderTargetManager* FHuaweiVRSDK::GetRenderTargetManager()
{
    LOGI("HvrLog: FHuaweiVRSDK::GetRenderTargetManager");
    return this;
}

//�����ӿ��� override
void FHuaweiVRSDK::SetupViewFamily(FSceneViewFamily& InViewFamily)
{
    LOGI("HvrLog: FHuaweiVRSDK::SetupViewFamily");
	InViewFamily.EngineShowFlags.MotionBlur = 0;
    InViewFamily.EngineShowFlags.HMDDistortion = false;
    InViewFamily.EngineShowFlags.SetScreenPercentage(true);
    InViewFamily.EngineShowFlags.ScreenPercentage = true;
    InViewFamily.EngineShowFlags.StereoRendering = IsStereoEnabled();
}

//�����ӿڣ������ã�override
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
    return GEngine && GEngine->IsStereoscopic3D(InViewport);//�������ж��Ƿ�Ӧ�û�Ծ��֡
}

//�Ƿ�Ӧ��ʹ��������Ⱦȥ��ȾĿ��
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
//������ȾĿ�����������ֵ��ʾ�Ƿ����ɹ�
bool FHuaweiVRSDK::AllocateRenderTargetTexture(uint32 Index, uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips,
    ETextureCreateFlags Flags, ETextureCreateFlags TargetableTextureFlags, FTexture2DRHIRef& OutTargetableTexture,
    FTexture2DRHIRef& OutShaderResourceTexture, uint32 NumSamples)
{
    LOGI("HvrLog: FHuaweiVRSDK::AllocateRenderTargetTexture, index = %u, SizeX, SizeY:(%u, %u), NumMips = %u, IsInGameThread = %d, IsInRenderingThread = %d, IsMultiviewState = %d ", Index, SizeX, SizeY, NumMips, IsInGameThread(), IsInRenderingThread(), IsMultiviewState);
    check(Index == 0);
	check(SizeX != 0 && SizeY != 0);
    // checking if rendering thread is suspended  �����Ⱦ�߳��Ƿ����
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

//game�߳�ʹ�õģ���ȡ��Ծ����Ⱦ����
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

//Ӧ����ͣ ί��
void FHuaweiVRSDK::ApplicationPauseDelegate()
{
    LOGI("HvrLog: FHuaweiVRSDK::ApplicationPauseDelegate");
    LOGI("FHuaweiVRSDK::ApplicationPauseDelegate");
}

//Ӧ�ü��� ί��
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

    HuaweiVRSDKMessageTable::init();//û������
#if PLATFORM_ANDROID
    LOGI("%s COMPILE TIME : " __DATE__ " " __TIME__, TCHAR_TO_ANSI(*HuaweiVRSDKUnrealVersion));
    HVR_SendMessage("pluginVersion", TCHAR_TO_ANSI(*HuaweiVRSDKUnrealVersion));
#endif
    CustomPresent = new FHuaweiVRCustomPresent();//�����Զ�����ʾ��

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
		RenderSizeX = HUAWEI_RT_WIDTH_MULTIVIEW_H;//����Ƕ��ӿڣ�����Ⱦ������Ϊ�̶�ֵ��1280*1280��
        RenderSizeY = HUAWEI_RT_WIDTH_MULTIVIEW_H;
	} else {
		RenderSizeX = width;
		RenderSizeY = height;
	}

}

//������Ⱦ֡��
void FHuaweiVRSDK::SetRenderFrameRate(int frameRate)
{
    LOGI("HvrLog: FHuaweiVRSDK::SetRenderFrameRate %d, current RenderFrameRate %d", frameRate, RenderFrameRate);
    if (RenderFrameRate != frameRate) {
        RenderFPSChanged = true;
    }
    RenderFrameRate = frameRate;
}

//������Ⱦ֡�ʣ�Ӧ��֡��
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

//�����Ƿ��ʼ��
bool FHuaweiVRSDK::IsInitialized() const
{
    LOGI("HvrLog: FHuaweiVRSDK::IsInitialized");
    return true;
}

//�����ӿ�size
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


//��ȡVR��Ϣ������֡�ʣ������ӿ�size
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

//����ɫ���
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