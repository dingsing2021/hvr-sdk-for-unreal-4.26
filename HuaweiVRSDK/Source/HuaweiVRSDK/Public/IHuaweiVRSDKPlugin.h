// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IHeadMountedDisplayModule.h"
//�����ģ�壬���û��������ģ�壬Unreal��ʶ�𲻵����ǵĲ��
/* *
 * The public interface to this module.  In most cases, this interface is only public to sibling modules
 * within this plugin.
 */
class IHuaweiVRSDKPlugin : public IHeadMountedDisplayModule {
public:
    /* *
     * Singleton-like access to this module's interface.  This is just for convenience!
     * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
     *
     * @return Returns singleton instance, loading the module on demand if needed
     */
    static inline IHuaweiVRSDKPlugin& Get()
    {
        return FModuleManager::LoadModuleChecked<IHuaweiVRSDKPlugin>("HuaweiVRSDK");
    }

    /* *
     * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
     *
     * @return True if the module is loaded and ready to use
     */
    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("HuaweiVRSDK");
    }
};