// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#if PLATFORM_ANDROID
#include "HuaweiVRSDKAPI.h"
#endif
/* *
 *
 */
#ifdef __cplusplus
extern "C" {
#endif
#if PLATFORM_ANDROID
JNI_METHOD void Java_com_epicgames_ue4_GameActivity_nativeOnCreate(JNIEnv* env, jobject gameActivity);
JNI_METHOD void Java_com_epicgames_ue4_GameActivity_nativeOnPause(JNIEnv* env, jobject gameActivity);
JNI_METHOD void Java_com_epicgames_ue4_GameActivity_nativeOnResume(JNIEnv* env, jobject gameActivity);
JNI_METHOD void Java_com_epicgames_ue4_GameActivity_nativeOnDestroy(JNIEnv* env, jobject gameActivity);
// JNI_METHOD void Java_com_epicgames_ue4_GameActivity_nativeSetMainActivityClass(JNIEnv* env, jobject
// gameActivityClass);

// static void FinishActivity(); // 422
void FinishActivity();

#endif
#ifdef __cplusplus
}
#endif
