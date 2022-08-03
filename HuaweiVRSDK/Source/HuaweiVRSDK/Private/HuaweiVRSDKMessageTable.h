// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/* *
 *
 */
class HuaweiVRSDKMessageTable {
public:
    static int init();
    static bool QueryMessage(int& type, int& priority, int& boxType, TMap<FString, FString>& message);

private:
    static bool CreateThermalMessage(TMap<FString, FString>& message);
    static bool CreatePhoneLowPowerMessage(TMap<FString, FString>& message);
    static bool CreateHelmetLowPowerMessage(TMap<FString, FString>& message);
    static bool CreateGeneralMessage(TMap<FString, FString>& message);
    static bool CreateSMSMessage(TMap<FString, FString>& message);
    static bool CreateNetWorkStateMessage(TMap<FString, FString>& message);

private:
    static FString Language;
};