#ifndef M_IMPORT_H
#define M_IMPORT_H

#include "module_base.h"

class MCart;
class MImport : public DModuleBase
{
    Q_OBJECT
public:
    explicit MImport(QObject *parent = nullptr);

    void startReceieNewRack();
    void stopReceiveNewRack();

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

private:
    void state_init();

    bool m_sensorUpdate;
    bool m_isReceiveNewEnable;

    QTimer *m_importTimer;
    enum class ImState {
        Idle,

        Check_New_Sample_Rack,
        WaitF_New_Sample_Rack,

        Send_Cart_GoTo_Import_Pos,
        WaitF_Cart_GoTo_Import_Pos_Ready,

        Check_New_Sample_Rack_1,
        WaitF_New_Sample_Rack_1,

        Send_Load_Rack_Cmd,
        WaitF_Load_Rack_Cmd_Done,

        New_Rack,

        Send_Load_Rack_Reset_Cmd,
        WaitF_Load_Rack_Reset_Cmd_Done,

        Error
    } m_imState;

    int m_checkSensorCount;
    int m_importFailCount;

    MCart *mCart;

private slots:
    void onImportTimer_slot();
};

#endif // M_IMPORT_H
