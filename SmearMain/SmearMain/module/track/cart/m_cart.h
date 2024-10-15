#ifndef M_CART_H
#define M_CART_H

#include "module_base.h"

enum Position {
    Pos_Origin = 0,

    Pos_Import,
    Pos_Scan,
    Pos_Test,

    Pos_Export
};

class CartCoord
{
public:
    static const int Width = 10;
    static QMap<QString, int> CoordBaseMap;

    CartCoord();
    explicit CartCoord(const QString &name);

    void setName(const QString &name) { m_name = name; }
    void setCoord(int x);
    void setRange(int to);
    QString getCoorStr();

    Position pos;

    int coord{0};
    int toCoord{0};
    int left{0};
    int right{0};

private:
    QString m_name;
};

class RtRack;
class MSampling;
class MExport;
class MCart : public DModuleBase
{
    Q_OBJECT
    friend class CartsManager;
public:
    explicit MCart(const QString &mid, const QString &userid, QObject *parent = nullptr);

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

    void suspend() { m_suspend = true; }
    void resume() { m_suspend = false; }
    void abort();

    void setSampling(MSampling *sampling) { mSampling = sampling; }
    void setExport(MExport *exp) { mExport = exp; }

    void setEnable(bool en) { m_isEnable = en; }
    bool isEnable() { return m_isEnable; }

    void setLock(bool lock) { mLock = lock; m_isLocked = false; }
    bool isLocked() { return m_isLocked; }

    bool isResetting() { return m_isResetting; }

    /* process */
    QSharedPointer<RtRack> newRack();
    QSharedPointer<RtRack> rack() { return mRack; }

    bool permitAlliedBack();

    /* request */
    virtual bool req_Reset() = 0;
    virtual bool req_PosImport() = 0;
    virtual bool req_PosScan(int pos) = 0;
    virtual bool req_PosTest(int pos) = 0;
    virtual bool req_PosExport() = 0;
    virtual bool req_PosLeft(int offset) = 0;
    virtual bool req_PosRight(int offset) = 0;

    void cmd_Reset();
    void cmd_PosImport();
    void cmd_PosScan(int pos);
    void cmd_PosTest(int pos);
    void cmd_PosExport();
    void cmd_PosLeft(int offset);
    void cmd_PosRight(int offset);

    /* command result */
    virtual bool isCmd_Reset_done() = 0;
    virtual bool isCmd_PosImport_done() = 0;
    virtual bool isCmd_PosScan_done() = 0;
    virtual bool isCmd_PosTest_done() = 0;
    virtual bool isCmd_PosExport_done() = 0;
    virtual bool isCmd_PosLeft_done() = 0;
    virtual bool isCmd_PosRight_done() = 0;

public slots:
    void onFunctionFinished_slot(const QString &api, const QJsonValue &resValue);

protected:
    virtual bool cmd_CReset() = 0;
    virtual bool cmd_CPosImport() = 0;
    virtual bool cmd_CPosScan(int pos) = 0;
    virtual bool cmd_CPosTest(int pos) = 0;
    virtual bool cmd_CPosExport() = 0;
    virtual bool cmd_CPosLeft(int pos) = 0;
    virtual bool cmd_CPosRight(int pos) = 0;

    virtual void updatePosition(const QString &api) = 0;
    virtual void onFunctionFinished(const QString &){}
    void setRackCoord(int coord);

private slots:
    void processTimer_slot();
    void cancelTimeoutTimer_slot();

protected:
    bool m_isHighPriority;
    CartCoord mCoord;
    enum RelativePos {
        RePos_None,
        RePos_Single,
        RePos_Front,
        RePos_Back
    } mRelatePos;

    MSampling *mSampling;
    MExport *mExport;

    bool m_suspend;
    bool m_isResetting;
    bool m_hasAvoidRequest;

private:
    QSharedPointer<RtRack> mRack = nullptr;

    bool mLock;
    bool m_isLocked;
    bool m_isEnable;
    bool m_isAborted;

    void state_init();

    QTimer *mMainTimer;
    enum class MainProcess {
        Idle,
        Wait_Another_Cart,

        Recycle_Sample,
        WaitF_1,

        Send_Sample,
        WaitF_2,

        Scan_Sample,
        WaitF_3,

        Recycle_Rack,
        WaitF_4,

        Avoid,
        WaitF_Avoid_Done,

        Reset,
        All_Finish
    } s_MainProcess;

    /* 1. Scan Barcode */
    int mScan_pos;
    QString m_scanBarCode;
    QSharedPointer<RtSample> mScanSample;
    enum class ScanState {
        Finish,
        Idle,

        Move_To_Scan_Pos,
        WaitF_Pos_Done,

        Sample_Exist_Check,
        WaitF_Sample_Exist_Check,

        Send_Scan_Cmd,
        WaitF_Scan_Cmd_Done,

        Done
    } s_Scan;
    void ScanBarcodeProcess();

    /* 2. Send Sample To Station */
    QSharedPointer<RtSample> mSendSample;
    enum class SendState {
        Finish,
        Idle,

        Move_To_Test_Pos,
        WaitF_Test_Pos_Done,

        Send_New_Sample_To_Station,
        WaitF_Send_Sample_To_Station_Done,

        Done
    } s_Send;
    void SendToStatProcess();

    /* 3. Receive Sample From Station */
    QSharedPointer<RtSample> mRecycleSample;
    enum class RecycleSampleState {
        Finish,
        Idle,

        Move_To_Station_Pos,
        WaitF_Move_To_Station_Pos,

        Send_Recycle_Sample,
        WaitF_Recyle_Sample_Done,

        Done
    } s_RecycleSample;
    void RecycleSampleFrStat();

    /* 4. Recycle Rack to Export */
    enum class RecycleRackState {
        Idle,

        Get_Export,

        Move_To_Export_Pos,
        WaitF_Export_Pos_Done,

        Req_Recycle_Rack,
        WaitF_Recycle_Rack_Done,

        Finish
    } s_RecycleRack;
    void RecycleRack();

    QTimer *m_timeoutTimer;
    int m_timeoutCount;

    int m_scanDuration;
};

#endif // M_CART_H

