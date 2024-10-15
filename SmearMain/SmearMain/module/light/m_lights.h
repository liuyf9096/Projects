#ifndef M_LIGHTS_H
#define M_LIGHTS_H

#include "module_base.h"

enum LightColor {
    LightColor_Red = 0,
    LightColor_Yellow = 1,
    LightColor_Green = 2
};

class MLights : public DModuleBase
{
    Q_OBJECT
public:
    static MLights *GetInstance();

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

    void setSlideStoreStart();
    void setStainOnlyStart();

    void setSlideStoreStop();
    void setStainOnlyStop();

    void setSlideStore1_Running(bool isRun) { m_slideStore1_running = isRun; }
    void setSlideStore2_Running(bool isRun) { m_slideStore2_running = isRun; }
    void setStainOnly1_Running(bool isRun) { m_stainOnly1_running = isRun; }
    void setStainOnly2_Running(bool isRun) { m_stainOnly2_running = isRun; }

    void handleMainlight(const QString &api, const QJsonObject &obj);

private:
    explicit MLights(const QString &mid, const QString &userid, QObject *parent = nullptr);

    void state_init();

    bool m_slideStore1_en;
    bool m_slideStore2_en;
    bool m_stainOnly1_en;
    bool m_stainOnly2_en;

    bool m_slideStore_update;
    bool m_stainOnly_update;

    bool m_slideStore1_running;
    bool m_slideStore2_running;
    bool m_stainOnly1_running;
    bool m_stainOnly2_running;

    enum LightStates {
        Red_On,
        Yellow_On,
        Green_On,
        Red_Blink,
        Yellow_Blink,
        Green_Blink,
        Breathe,
        Off,
        Unknown
    };

    LightStates mainlight_State, mainlight_Last;
    LightStates slideStore1_State, slideStore1_Last;
    LightStates slideStore2_State, slideStore2_Last;
    LightStates stainOnly1_State, stainOnly1_Last;
    LightStates stainOnly2_State, stainOnly2_Last;

    void slideStoreProcess();
    void stainOnlyProcess();

    void updateMainLight();
    void updateSlideStoreLight();
    void updateStainOnlyLight();
};

#endif // M_LIGHTS_H
