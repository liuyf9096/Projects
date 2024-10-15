#include "m_lights.h"
#include "f_common.h"

MLights *MLights::GetInstance()
{
    static MLights *instance = nullptr;
    if (instance == nullptr) {
        instance = new MLights("lights", "lights");
    }
    return instance;
}

MLights::MLights(const QString &mid, const QString &userid, QObject *parent)
    : DModuleBase(mid, userid, parent)
{
    mLabel = "lights";

    state_init();

    m_slideStore1_en = FCommon::GetInstance()->getConfigValue("lights", "slide_store_1_en").toBool();
    m_slideStore2_en = FCommon::GetInstance()->getConfigValue("lights", "slide_store_2_en").toBool();
    m_stainOnly1_en = FCommon::GetInstance()->getConfigValue("lights", "stain_only1_en").toBool();
    m_stainOnly2_en = FCommon::GetInstance()->getConfigValue("lights", "stain_only2_en").toBool();

    auto smear = RtDeviceManager::GetInstance()->smear();
    connect(smear, &DSmear::sonserInfo_signal, this, [&](){
        if (m_slideStore_update == true) {
            slideStoreProcess();
        }
    });

    auto stain = RtDeviceManager::GetInstance()->stain();
    connect(stain, &DStain::sonserInfo_signal, this, [&](){
        if (m_stainOnly_update == true) {
            stainOnlyProcess();
        }
    });
}

void MLights::state_init()
{
    m_slideStore1_running = false;
    m_slideStore2_running = false;
    m_stainOnly1_running = false;
    m_stainOnly2_running = false;

    m_slideStore_update = false;
    m_stainOnly_update = false;

    mainlight_State = Green_On;
    mainlight_Last = Unknown;

    slideStore1_State = Off;
    slideStore1_Last = Unknown;

    slideStore2_State = Off;
    slideStore2_Last = Unknown;

    stainOnly1_State = Off;
    stainOnly1_Last = Unknown;

    stainOnly2_State = Off;
    stainOnly2_Last = Unknown;
}

void MLights::start()
{
    setSlideStoreStart();
    setStainOnlyStart();

    updateMainLight();
    updateSlideStoreLight();
    updateStainOnlyLight();
}

void MLights::reset()
{
    state_init();
}

void MLights::stop()
{
    setSlideStoreStop();
    setStainOnlyStop();
}

void MLights::setSlideStoreStart()
{
    if (m_slideStore1_en || m_slideStore2_en) {
        dev->smear()->setCheckSensorEnable(mModuleId, true);
        m_slideStore_update = true;
    }
}

void MLights::setStainOnlyStart()
{
    if (m_stainOnly1_en || m_stainOnly2_en) {
        dev->stain()->setCheckSensorEnable(mModuleId, true);
        m_stainOnly_update = true;
    }
}

void MLights::setSlideStoreStop()
{
    dev->smear()->setCheckSensorEnable(false);
    m_slideStore_update = false;
    slideStore1_State = Off;
    slideStore2_State = Off;
    slideStoreProcess();
}

void MLights::setStainOnlyStop()
{
    dev->stain()->setCheckSensorEnable(false);
    m_stainOnly_update = false;
    stainOnly1_State = Off;
    stainOnly2_State = Off;
    stainOnlyProcess();
}

void MLights::handleMainlight(const QString &api, const QJsonObject &obj)
{
    QString color = obj.value("color").toString();

    if (api == "SetMainLishtOn") {
        if (color == "red") {
            mainlight_State = Red_On;
        } else if (color == "yellow") {
            mainlight_State = Yellow_On;
        } else if (color == "green") {
            mainlight_State = Green_On;
        }
    } else if (api == "SetMainLishtBlink") {
        if (color == "red") {
            mainlight_State = Red_Blink;
        } else if (color == "yellow") {
            mainlight_State = Yellow_Blink;
        } else if (color == "green") {
            mainlight_State = Green_Blink;
        }
    } else if (api == "SetMainLishtBreathe") {
        mainlight_State = Breathe;
    }

    updateMainLight();
}

void MLights::slideStoreProcess()
{
    if (m_slideStore1_en == true) {
        if (dev->smear()->checkSensorValue("SlideStore1_exist") == true) {
            if (dev->smear()->checkSensorValue("SlideStore1_remain") == true) {
                if (m_slideStore1_running == false) {
                    slideStore1_State = Green_On;
                } else {
                    slideStore1_State = Green_Blink;
                }
            } else {
                slideStore1_State = Red_On;
            }
        } else {
            slideStore1_State = Off;
        }
    }

    if (m_slideStore2_en == true) {
        if (dev->smear()->checkSensorValue("SlideStore2_exist") == true) {
            if (dev->smear()->checkSensorValue("SlideStore2_remain") == true) {
                if (m_slideStore2_running == false) {
                    slideStore2_State = Green_On;
                } else {
                    slideStore2_State = Green_Blink;
                }
            } else {
                slideStore2_State = Red_On;
            }
        } else {
            slideStore2_State = Off;
        }
    }

    updateSlideStoreLight();
}

void MLights::stainOnlyProcess()
{
    if (m_stainOnly1_en == true) {
        if (dev->stain()->checkSensorValue("StainOnly1_exist") == true) {
            if (m_stainOnly1_running == false) {
                stainOnly1_State = Green_On;
            } else {
                stainOnly1_State = Green_Blink;
            }
        } else {
            stainOnly1_State = Off;
        }
    }
    if (m_stainOnly2_en == true) {
        if (dev->stain()->checkSensorValue("StainOnly2_exist") == true) {
            if (m_stainOnly2_running == false) {
                stainOnly2_State = Green_On;
            } else {
                stainOnly2_State = Green_Blink;
            }
        } else {
            stainOnly2_State = Off;
        }
    }

    updateStainOnlyLight();
}

void MLights::updateMainLight()
{
    if (mainlight_State != mainlight_Last) {
        if (mainlight_State == Red_On) {
            dev->sample()->cmd_MainLight_On(LightColor_Red);
        } else if (mainlight_State == Yellow_On) {
            dev->sample()->cmd_MainLight_On(LightColor_Yellow);
        } else if (mainlight_State == Green_On) {
            dev->sample()->cmd_MainLight_On(LightColor_Green);
        } else if (mainlight_State == Red_Blink) {
            dev->sample()->cmd_MainLight_Blink(LightColor_Red);
        } else if (mainlight_State == Yellow_Blink) {
            dev->sample()->cmd_MainLight_Blink(LightColor_Yellow);
        } else if (mainlight_State == Green_Blink) {
            dev->sample()->cmd_MainLight_Blink(LightColor_Green);
        } else if (mainlight_State == Breathe) {
            dev->sample()->cmd_MainLight_Breathe();
        } else if (mainlight_State == Off) {
            dev->sample()->cmd_MainLight_Off();
        }
        mainlight_Last = mainlight_State;
    }
}

void MLights::updateSlideStoreLight()
{
    if (slideStore1_State != slideStore1_Last) {
        if (slideStore1_State == Red_On) {
            dev->stain()->cmd_SlideStoreLight1_On(LightColor_Red);
        } else if (slideStore1_State == Yellow_On) {
            dev->stain()->cmd_SlideStoreLight1_On(LightColor_Yellow);
        } else if (slideStore1_State == Green_On) {
            dev->stain()->cmd_SlideStoreLight1_On(LightColor_Green);
        } else if (slideStore1_State == Red_Blink) {
            dev->stain()->cmd_SlideStoreLight1_Blink(LightColor_Red);
        } else if (slideStore1_State == Yellow_Blink) {
            dev->stain()->cmd_SlideStoreLight1_Blink(LightColor_Yellow);
        } else if (slideStore1_State == Green_Blink) {
            dev->stain()->cmd_SlideStoreLight1_Blink(LightColor_Green);
        } else if (slideStore1_State == Off) {
            dev->stain()->cmd_SlideStoreLight1_Off();
        }
        slideStore1_Last = slideStore1_State;
    }

    if (slideStore2_State != slideStore2_Last) {
        if (slideStore2_State == Red_On) {
            dev->stain()->cmd_SlideStoreLight2_On(LightColor_Red);
        } else if (slideStore2_State == Yellow_On) {
            dev->stain()->cmd_SlideStoreLight2_On(LightColor_Yellow);
        } else if (slideStore2_State == Green_On) {
            dev->stain()->cmd_SlideStoreLight2_On(LightColor_Green);
        } else if (slideStore2_State == Red_Blink) {
            dev->stain()->cmd_SlideStoreLight2_Blink(LightColor_Red);
        } else if (slideStore2_State == Yellow_Blink) {
            dev->stain()->cmd_SlideStoreLight2_Blink(LightColor_Yellow);
        } else if (slideStore2_State == Green_Blink) {
            dev->stain()->cmd_SlideStoreLight2_Blink(LightColor_Green);
        } else if (slideStore2_State == Off) {
            dev->stain()->cmd_SlideStoreLight2_Off();
        }
        slideStore2_Last = slideStore2_State;
    }
}

void MLights::updateStainOnlyLight()
{
    if (stainOnly1_State != stainOnly1_Last) {
        if (stainOnly1_State == Red_On) {
            dev->stain()->cmd_StainOnlyLight1_On(LightColor_Red);
        } else if (stainOnly1_State == Yellow_On) {
            dev->stain()->cmd_StainOnlyLight1_On(LightColor_Yellow);
        } else if (stainOnly1_State == Green_On) {
            dev->stain()->cmd_StainOnlyLight1_On(LightColor_Green);
        } else if (stainOnly1_State == Red_Blink) {
            dev->stain()->cmd_StainOnlyLight1_Blink(LightColor_Red);
        } else if (stainOnly1_State == Yellow_Blink) {
            dev->stain()->cmd_StainOnlyLight1_Blink(LightColor_Yellow);
        } else if (stainOnly1_State == Green_Blink) {
            dev->stain()->cmd_StainOnlyLight1_Blink(LightColor_Green);
        } else if (stainOnly1_State == Off) {
            dev->stain()->cmd_StainOnlyLight1_Off();
        }
        stainOnly1_Last = stainOnly1_State;
    }

    if (stainOnly2_State != stainOnly2_Last) {
        if (stainOnly2_State == Red_On) {
            dev->stain()->cmd_StainOnlyLight2_On(LightColor_Red);
        } else if (stainOnly2_State == Yellow_On) {
            dev->stain()->cmd_StainOnlyLight2_On(LightColor_Yellow);
        } else if (stainOnly2_State == Green_On) {
            dev->stain()->cmd_StainOnlyLight2_On(LightColor_Green);
        } else if (stainOnly2_State == Red_Blink) {
            dev->stain()->cmd_StainOnlyLight2_Blink(LightColor_Red);
        } else if (stainOnly2_State == Yellow_Blink) {
            dev->stain()->cmd_StainOnlyLight2_Blink(LightColor_Yellow);
        } else if (stainOnly2_State == Green_Blink) {
            dev->stain()->cmd_StainOnlyLight2_Blink(LightColor_Green);
        } else if (stainOnly2_State == Off) {
            dev->stain()->cmd_StainOnlyLight2_Off();
        }
        stainOnly2_Last = stainOnly2_State;
    }
}
