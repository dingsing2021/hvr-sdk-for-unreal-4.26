// Fill out your copyright notice in the Description page of Project Settings.

#include "HuaweiVRSDKJNI.h"
#include "HuaweiVRSDK.h"

#if PLATFORM_ANDROID

void Java_com_epicgames_ue4_GameActivity_nativeOnCreate(JNIEnv* env, jobject gameActivity)
{
    LOGI("Java_com_epicgames_ue4_GameActivity_nativeOnCreate");
}

void Java_com_epicgames_ue4_GameActivity_nativeOnPause(JNIEnv* env, jobject gameActivity)
{
    LOGI("Java_com_epicgames_ue4_GameActivity_nativeOnPause");
    FHuaweiVRSDK* huaweiVRSDK = FHuaweiVRSDK::GetHuaweiVRSDK();
    if (huaweiVRSDK) {
        FHuaweiVRCustomPresent* customPresent = huaweiVRSDK->GetCustomPresent();
        if (customPresent) {
            customPresent->MainThreadRunningNotify(false);
        }
    }
}

void Java_com_epicgames_ue4_GameActivity_nativeOnResume(JNIEnv* env, jobject gameActivity)
{
    LOGI("Java_com_epicgames_ue4_GameActivity_nativeOnResume");
    FHuaweiVRSDK* huaweiVRSDK = FHuaweiVRSDK::GetHuaweiVRSDK();
    if (huaweiVRSDK) {
        FHuaweiVRCustomPresent* customPresent = huaweiVRSDK->GetCustomPresent();
        if (customPresent) {
            customPresent->MainThreadRunningNotify(true);
        }
    }
}

void Java_com_epicgames_ue4_GameActivity_nativeOnDestroy(JNIEnv* env, jobject gameActivity)
{
    LOGI("Java_com_epicgames_ue4_GameActivity_nativeOnDestroy");
}

void FinishActivity()
{
    JNIEnv* env = NULL;
    jmethodID finishActivityMethod = NULL;

    if (GJavaVM) {
        GJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
        if (env) {
            jclass activityClass = env->FindClass("android/app/Activity");
            if (activityClass) {
                finishActivityMethod = env->GetMethodID(activityClass, "finish", "()V");
                env->DeleteLocalRef(activityClass);
            } else {
                LOGE("Java_com_epicgames_ue4_GameActivity_nativeOnCreate find activity class failed!");
            }

            if (finishActivityMethod) {
                env->CallVoidMethod(FJavaWrapper::GameActivityThis, finishActivityMethod);
            } else {
                LOGI("finishActivityMethod is NULL!");
            }
        } else {
            LOGI("JNIEnv is NULL!");
        }
    } else {
        LOGI("GJavaVM is NULL!");
    }
}

#endif
