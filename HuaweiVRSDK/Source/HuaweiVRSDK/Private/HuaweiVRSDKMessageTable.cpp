// Fill out your copyright notice in the Description page of Project Settings.

#include "HuaweiVRSDKMessageTable.h"
#include "HuaweiVRSDKAPI.h"
#include "HuaweiVRSDK.h"

static const FString m_text_thermalWarningContent_zh_0 =
    TEXT("手机温度偏高。为保证使用体验，建议将手机放置在散热较好之处");
static const FString m_text_thermalWarningContent_zh_1 =
    TEXT("手机温度过高。建议将手机放置在散热较好处，或等待手机降温至正常");
static const FString m_text_thermalWarningContent_zh_2 = TEXT("手机温度过高，");
static const FString m_text_thermalWarningContent_zh_3 =
    TEXT("秒后将退出VR模式。退出后，请等待手机降温至正常之后再体验VR");
static const FString m_text_thermalWarningContent_en_0 =
    TEXT("Your phone is too hot. Please move it to a well-ventilated place to ensure optimal performance.");
static const FString m_text_thermalWarningContent_en_1 =
    TEXT("Your phone is too hot. Please move it to a well-ventilated place or wait for it to cool down.");
static const FString m_text_thermalWarningContent_en_2 = TEXT("Your phone is too hot. VR mode will exit in ");
static const FString m_text_thermalWarningContent_en_3 =
    TEXT(" second. Please wait for the phone to cool down before continuing to use VR.");

static const FString m_text_closeDialogTips_zh = TEXT("按下触控板可关闭提示");
static const FString m_text_closeDialogTips_en = TEXT("Press the touch pad to close this tip");

static const FString m_text_chargeTitle_zh = TEXT("已连接充电器");
static const FString m_text_chargeMsg_zh = TEXT("充电过程可能影响您的VR体验，建议取下手机充电");
static const FString m_text_chargeTitle_en = TEXT("Charger connected");
static const FString m_text_chargeMsg_en = TEXT("Charging may affect your VR experience, please remove phone");

static const FString m_text_connectVRGlass_zh = TEXT("若要运行该应用，请先将手机连接至华为VR");
static const FString m_text_connectVRGlass_en = TEXT("To open this app, please connect your phone to Huawei VR.");
static const FString m_text_PhoneLowPowerTitle_zh = TEXT("电量低");
static const FString m_text_PhoneLowPowerTitle_en = TEXT("Low battery");

static const FString m_text_PhoneLowPowerMsg_zh_0 = TEXT("手机电量不足10%，请连接充电器");
static const FString m_text_PhoneLowPowerMsg_en_0 =
    TEXT("Phone battery level is below 10%, please connect the charger.");
static const FString m_text_PhoneLowPowerMsg_zh_1 = TEXT("手机电量不足5%，即将自动关机");
static const FString m_text_PhoneLowPowerMsg_en_1 =
    TEXT("Phone battery level is below 5%. It will automatically power off in seconds.");

static const FString m_text_HelmetLowPowerTitle_zh = TEXT("电量低");
static const FString m_text_HelmetLowPowerTitle_en = TEXT("Low battery");
static const FString m_text_HelmetLowPowerMsg_zh_0 = TEXT("头盔电量不足20%，请连接充电器");
static const FString m_text_HelmetLowPowerMsg_en_0 =
    TEXT("Headset battery level is below 20%, please connect the charger.");
static const FString m_text_HelmetLowPowerMsg_zh_1 = TEXT("头盔电量过低，即将自动关机");
static const FString m_text_HelmetLowPowerMsg_en_1 =
    TEXT("Headset battery level is too low. It will automatically power off in seconds.");

static const FString m_text_wifiTitle_zh_0 = TEXT("无线网络已断开");
static const FString m_text_wifiTitle_zh_1 = TEXT("无线网络已连接");
static const FString m_text_wifiTitle_en_0 = TEXT("Disconnected from Wi-Fi");
static const FString m_text_wifiTitle_en_1 = TEXT("Connected to Wi-Fi");

static const FString HuaweiVRMessageKey_Title = TEXT("title");
static const FString HuaweiVRMessageKey_Content = TEXT("content");
static const FString HuaweiVRMessageKey_Tips = TEXT("tips");
static const FString HuaweiVRMessageKey_Extention = TEXT("extention");

FString HuaweiVRSDKMessageTable::Language = FGenericPlatformMisc::GetDefaultLanguage();

static int HuaweiVRSDKNativeMessageReceiver(const char* key, const char* value)
{
    if (NULL == key) {
        LOGE("HuaweiVRSDKNativeMessageReceiver key is NULL");
        return -1;
    }

    if (!strcmp(key, "SetRenderFrameRate")) {
        int frameRate = ((value == NULL) ? 0 : atoi(value));
        LOGI("HuaweiVRSDKNativeMessageReceiver SetRenderFrameRate %d", frameRate);
        if (frameRate > 0) {
            FHuaweiVRSDK::SetRenderFrameRate(frameRate);
        }
    } else if (!strcmp(key, "SetRenderRatio")) {
        int width = ((value == NULL) ? 0 : atoi(value));
        int height = ((value == NULL) ? 0 : atoi(value));
        LOGI("HuaweiVRSDKNativeMessageReceiver SetRenderRatio width=%d, height=%d", width, height);
        if (width > 0 && height > 0) {
            FHuaweiVRSDK::SetRenderSize(width, height);
        }
    }
    return 0;
}

int HuaweiVRSDKMessageTable::init()
{
#if PLATFORM_ANDROID
    HVR_RegistReceiveMessageCallback(HuaweiVRSDKNativeMessageReceiver);
#endif
    return 0;
}

//获取队列信息， 类型 优先级 盒子类型 具体信息
bool HuaweiVRSDKMessageTable::QueryMessage(int& type, int& priority, int& boxType, TMap<FString, FString>& message)
{
    bool ret = false;
    int notification = -1;

    type = -1;
    message.Empty();

    // The value of static initialized Language is empty, so it needs to be assigned again
    //  静态初始化语言的值为空，需要重新赋值
    Language = FGenericPlatformMisc::GetDefaultLanguage();

#if PLATFORM_ANDROID
    HVR_GetMsgIdx(type);//向下获取信息类型
#endif

#if PLATFORM_ANDROID
    notification = HVR_Android_GetNotifyEvent();//获取通知事件 这个可能要对照SDK看一下
#endif
    if ((notification & (int)NotifyEvent_ThermalWarning) > 0) {
        type = (int)NotifyEvent_ThermalWarning;//温度警告
    }

    if ((notification & (int)NotifyEvent_PhoneLowPower) > 0) {
        type = (int)NotifyEvent_PhoneLowPower;//电量低
    }

    if ((notification & (int)NotifyEvent_HelmetLowPower) > 0) {
        type = (int)NotifyEvent_HelmetLowPower;//头盔电量低
    }

    if ((notification & (int)NotifyEvent_GeneralMessage) > 0) {
        type = (int)NotifyEvent_GeneralMessage;//通用信息
    }

    switch (type) {
        case (int)NotifyEvent_ThermalWarning:
            ret = CreateThermalMessage(message);//获取具体信息
            break;
        case (int)NotifyEvent_PhoneLowPower:
            ret = CreatePhoneLowPowerMessage(message);
            break;
        case (int)NotifyEvent_HelmetLowPower:
            ret = CreateHelmetLowPowerMessage(message);
            break;
        case (int)NotifyEvent_GeneralMessage:
            ret = CreateGeneralMessage(message);
            break;
        case (int)NOTIFICATION_GENERAL_MESSAGE:
            ret = CreateGeneralMessage(message);
            break;
        case (int)NotifyEvent_NewSMS:
            ret = CreateSMSMessage(message);
            break;
        case (int)NOTIFICATION_SMS_MESSAGE:
            ret = CreateSMSMessage(message);
            break;
        case (int)NotifyEvent_NetWorkState:
            ret = CreateNetWorkStateMessage(message);
            break;
        case (int)NOTIFICATION_NETWORKCHANGE_MESSAGE:
            ret = CreateNetWorkStateMessage(message);
            break;
        default:
            break;
    }

    return ret;
}

bool HuaweiVRSDKMessageTable::CreateThermalMessage(TMap<FString, FString>& message)
{
    bool ret = false;

#if PLATFORM_ANDROID
    int level = 0, frameRate = 0, ratioWidth = 0, ratioHeight = 0;
    if (0 == HVR_Android_GetThermalWarningInfo(level, frameRate, ratioWidth, ratioHeight)) {//向SDK获取温警等级，当前帧率，宽高比例
        if (1 == level) {
            if (Language.StartsWith("zh_CN")) {
                message.Add(HuaweiVRMessageKey_Content, m_text_thermalWarningContent_zh_0);
                message.Add(HuaweiVRMessageKey_Tips, m_text_closeDialogTips_zh);
            } else /* if (Language.StartsWith("en")) */ {
                message.Add(HuaweiVRMessageKey_Content, m_text_thermalWarningContent_en_0);
                message.Add(HuaweiVRMessageKey_Tips, m_text_closeDialogTips_en);
            } /* else {
                //Todo
            } */
        } else if (2 == level) {
            if (Language.StartsWith("zh_CN")) {
                message.Add(HuaweiVRMessageKey_Content, m_text_thermalWarningContent_zh_1);
                message.Add(HuaweiVRMessageKey_Tips, m_text_closeDialogTips_zh);
            } else /* if (Language.StartsWith("en")) */ {
                message.Add(HuaweiVRMessageKey_Content, m_text_thermalWarningContent_en_1);
                message.Add(HuaweiVRMessageKey_Tips, m_text_closeDialogTips_en);
            } /* else {
                //Todo
            } */
        } else if (3 == level) {
            FString content = "";
            if (Language.StartsWith("zh_CN")) {
                message.Add(HuaweiVRMessageKey_Content, content.Append(m_text_thermalWarningContent_zh_2)
                                                            .Append("10")
                                                            .Append(m_text_thermalWarningContent_zh_3));
                message.Add(HuaweiVRMessageKey_Tips, m_text_closeDialogTips_zh);
            } else /* if (Language.StartsWith("en")) */ {
                message.Add(HuaweiVRMessageKey_Content, content.Append(m_text_thermalWarningContent_en_2)
                                                            .Append("10")
                                                            .Append(m_text_thermalWarningContent_en_3));
                message.Add(HuaweiVRMessageKey_Tips, m_text_closeDialogTips_en);
            } /* else {
                //Todo
            } */
            message.Add(HuaweiVRMessageKey_Extention, TEXT("AppQuit"));
        }

        LOGI("CreateThermalMessage level=%d, language=%s", level, TCHAR_TO_ANSI(*Language));
        // message.Add(HuaweiVRMessageKey_Extention, FString::Printf__VA(L"%d##%d##%d##%d", level, frameRate,
        // ratioWidth, ratioHeight));
        ret = true;
    }

#endif
    return ret;
}

bool HuaweiVRSDKMessageTable::CreatePhoneLowPowerMessage(TMap<FString, FString>& message)
{
    bool ret = false;

#if PLATFORM_ANDROID
    int flag = -1;
    if (0 == HVR_Android_GetLowpowerFlag(0, flag)) {
        LOGI("PhoneLowPowerMessage flag=%d", flag);
        if (0 == flag) {
            if (Language.StartsWith("zh_CN")) {
                message.Add(HuaweiVRMessageKey_Title, m_text_PhoneLowPowerTitle_zh);
                message.Add(HuaweiVRMessageKey_Content, m_text_PhoneLowPowerMsg_zh_0);
            } else /* if (Language.StartsWith("en")) */ {
                message.Add(HuaweiVRMessageKey_Title, m_text_PhoneLowPowerTitle_en);
                message.Add(HuaweiVRMessageKey_Content, m_text_PhoneLowPowerMsg_en_0);
            } /* else {
                //Todo
            } */
        } else if (1 == flag) {
            if (Language.StartsWith("zh_CN")) {
                message.Add(HuaweiVRMessageKey_Title, m_text_PhoneLowPowerTitle_zh);
                message.Add(HuaweiVRMessageKey_Content, m_text_PhoneLowPowerMsg_zh_1);
            } else /* if (Language.StartsWith("en")) */ {
                message.Add(HuaweiVRMessageKey_Title, m_text_PhoneLowPowerTitle_en);
                message.Add(HuaweiVRMessageKey_Content, m_text_PhoneLowPowerMsg_en_1);
            } /* else {
                //Todo
            } */
        }
        message.Add(HuaweiVRMessageKey_Extention, FString::FromInt(flag));
        ret = true;
    }
#endif
    return ret;
}

bool HuaweiVRSDKMessageTable::CreateHelmetLowPowerMessage(TMap<FString, FString>& message)
{
    bool ret = false;

#if PLATFORM_ANDROID
    int flag = -1;
    if (0 == HVR_Android_GetLowpowerFlag(1, flag)) {
        LOGI("HelmetLowPowerMessage flag=%d", flag);
        if (0 == flag) {
            if (Language.StartsWith("zh_CN")) {
                message.Add(HuaweiVRMessageKey_Title, m_text_HelmetLowPowerTitle_zh);
                message.Add(HuaweiVRMessageKey_Content, m_text_HelmetLowPowerMsg_zh_0);
            } else /* if (Language.Equals("en")) */ {
                message.Add(HuaweiVRMessageKey_Title, m_text_HelmetLowPowerTitle_en);
                message.Add(HuaweiVRMessageKey_Content, m_text_HelmetLowPowerMsg_en_0);
            } /* else {
                //Todo
            } */
        } else if (1 == flag) {
            if (Language.StartsWith("zh_CN")) {
                message.Add(HuaweiVRMessageKey_Title, m_text_HelmetLowPowerTitle_zh);
                message.Add(HuaweiVRMessageKey_Content, m_text_HelmetLowPowerMsg_zh_1);
            } else /* if (Language.Equals("en")) */ {
                message.Add(HuaweiVRMessageKey_Title, m_text_HelmetLowPowerTitle_en);
                message.Add(HuaweiVRMessageKey_Content, m_text_HelmetLowPowerMsg_en_1);
            } /* else {
                //Todo
            } */
        }
        message.Add(HuaweiVRMessageKey_Extention, FString::FromInt(flag));
        ret = true;
    }
#endif
    return ret;
}

bool HuaweiVRSDKMessageTable::CreateGeneralMessage(TMap<FString, FString>& message)
{
    bool ret = false;

    FString strPackageName = "";
    FString strTitle = "";
    FString strContent = "";

#if PLATFORM_ANDROID
    char* packageName = HVR_GetNotifiPackage();
    if (packageName) {
        strPackageName = FString(UTF8_TO_TCHAR(packageName));
    }
    char* title = HVR_GetNotifiTitle();
    if (title) {
        strTitle = FString(UTF8_TO_TCHAR(title));
    }
    char* content = HVR_GetNotifiContent();
    if (content) {
        strContent = FString(UTF8_TO_TCHAR(content));
    }

    if (packageName) {
        message.Add(HuaweiVRMessageKey_Title, strTitle);
        message.Add(HuaweiVRMessageKey_Content, strContent);
        message.Add(HuaweiVRMessageKey_Extention, packageName);
    }
    LOGI("CreateGeneralMessage");

    ret = true;
#endif

    return ret;
}

bool HuaweiVRSDKMessageTable::CreateSMSMessage(TMap<FString, FString>& message)
{
    bool ret = false;
    FString strSmsPhoneNumber = "";
    FString strSmsContactName = "";
    FString strSmsContent = "";

#if PLATFORM_ANDROID
    char* phoneNumber = HVR_GetSmsNumber();
    if (phoneNumber) {
        strSmsPhoneNumber = FString(UTF8_TO_TCHAR(phoneNumber));
    }
    char* contactName = HVR_GetSmsContactName();
    if (contactName) {
        strSmsContactName = FString(UTF8_TO_TCHAR(contactName));
    }
    char* content = HVR_GetSmsMsg();
    if (content) {
        strSmsContent = FString(UTF8_TO_TCHAR(content));
    }

    message.Add(HuaweiVRMessageKey_Title, strSmsContactName.Equals("") ? strSmsPhoneNumber : strSmsContactName);
    message.Add(HuaweiVRMessageKey_Content, strSmsContent);

    LOGI("CreateSMSMessage");

    ret = true;

#endif
    return ret;
}

bool HuaweiVRSDKMessageTable::CreateNetWorkStateMessage(TMap<FString, FString>& message)
{
    bool ret = false;

#if PLATFORM_ANDROID
    bool isConnected = false;
    if (HVR_GetWifiMsg(isConnected)) {
        if (isConnected) {
            if (Language.StartsWith("zh_CN")) {
                message.Add(HuaweiVRMessageKey_Title, m_text_wifiTitle_zh_1);
            } else /* if (Language.Equals("en")) */ {
                message.Add(HuaweiVRMessageKey_Title, m_text_wifiTitle_en_1);
            } /* else {
                //Todo
            } */
            message.Add(HuaweiVRMessageKey_Extention, "connected");
        } else {
            if (Language.StartsWith("zh_CN")) {
                message.Add(HuaweiVRMessageKey_Title, m_text_wifiTitle_zh_0);
            } else /* if (Language.Equals("en")) */ {
                message.Add(HuaweiVRMessageKey_Title, m_text_wifiTitle_en_0);
            } /* else {
                //Todo
            } */
            message.Add(HuaweiVRMessageKey_Extention, "disconnected");
        }

        ret = true;
    }
#endif
    return ret;
}