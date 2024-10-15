#ifndef M_STAIN_CART_H
#define M_STAIN_CART_H

#include "module_base.h"
#include "sample/rt_sample.h"

class StainImport;
class MStainCart : public DModuleBase
{
    Q_OBJECT
public:
    explicit MStainCart(const QString &mid, QObject *parent = nullptr);

    void setStainImport(StainImport *import) { mImport = import; }

    bool isIdle();
    bool isLoaded();

    bool receiveNewSlide(QSharedPointer<RtSlide> slide);
    bool takeOutSlide(const QString &sid);

    void setHeaterParams(const QJsonObject &obj);

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

private:
    void state_init();

    StainImport *mImport;
    int m_drytime_sec;
    bool m_isHeaterEnable;

    QSharedPointer<RtSlide> m_slide = nullptr;

    QString m_api;

    QTimer *mProcessTimer;
    enum class Process {
        Idle,

        Send_Move_To_Stain_Cmd,
        WaitF_Move_To_Stain_Done,

        WaitF_Unload_Slide_Done,

        Send_Reset_Cmd,
        WaitF_Reset_Done,

        Finish
    } s_state;

private slots:
    void onProcessTimer_slot();
};

#endif // M_STAIN_CART_H
