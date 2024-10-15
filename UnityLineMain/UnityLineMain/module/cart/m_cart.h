#ifndef M_CART_H
#define M_CART_H

#include "module_base.h"

enum Position {
    Pos_Origin = 0,

    Pos_Import,
    Pos_Scan_1,
    Pos_Scan_RackCode_1,
    Pos_Test_1,

    Pos_Exit_E1,
    Pos_Scan_2,
    Pos_Scan_RackCode_2,
    Pos_Test_2,

    Pos_Scan_3,
    Pos_Scan_RackCode_3,
    Pos_Test_3,
    Pos_Exit_E2
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
class MStation;
class StationManager;
class MExport;
class MExport1;
class MExport2;
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

    void setExit1(MExport *e1) { mE1 = e1; }
    void setExit2(MExport *e2) { mE2 = e2; }

    void setEnable(bool en) { m_isEnable = en; }
    bool isEnable() { return m_isEnable; }

    void setLock(bool lock) { mLock = lock; m_isLocked = false; }
    bool isLocked() { return m_isLocked; }

    bool isResetting() { return m_isResetting; }

    /* process */
    void newRack();
    QSharedPointer<RtRack> rack() { return mRack;}

    bool permitAlliedBack();

    /* request */
    virtual bool req_Reset() = 0;
    virtual bool req_PosImport() = 0;
    virtual bool req_PosSxScanRackCode(const QString &sid) = 0;
    virtual bool req_PosSxScan(const QString &sid, int pos) = 0;
    virtual bool req_PosSxTest(const QString &sid, int pos) = 0;
    virtual bool req_PosExitEx(const QString &eid) = 0;
    virtual bool req_PosLeft(int offset) = 0;
    virtual bool req_PosRight(int offset) = 0;

    void cmd_Reset();
    void cmd_PosImport();
    void cmd_PosSxScanRackCode(const QString &sid);
    void cmd_PosSxScan(const QString &sid, int pos);
    void cmd_PosSxTest(const QString &sid, int pos);
    void cmd_PosExit_Ex(const QString &eid);
    void cmd_PosLeft(int offset);
    void cmd_PosRight(int offset);
    void cmd_Scan_Open(int index, int timeout);

    /* command result */
    virtual bool isCmd_Reset_done() = 0;
    virtual bool isCmd_PosImport_done() = 0;
    virtual bool isCmd_PosScanRackCode_done(const QString &sid) = 0;
    virtual bool isCmd_PosScan_done(const QString &sid) = 0;
    virtual bool isCmd_PosTest_done(const QString &sid) = 0;
    virtual bool isCmd_PosExit_Ex_done(const QString &eid) = 0;
    virtual bool isCmd_PosLeft_done() = 0;
    virtual bool isCmd_PosRight_done() = 0;
    bool isScanOpen_Done(int index = 1);

public slots:
    void onFunctionFinished_slot(const QString &api, const QJsonValue &resValue);

protected:
    virtual bool cmd_CReset() = 0;
    virtual bool cmd_CPosImport() = 0;
    virtual bool cmd_CPosScanRackCode(const QString &sid) = 0;
    virtual bool cmd_CPosSxScan(const QString &sid, int pos) = 0;
    virtual bool cmd_CPosSxTest(const QString &sid, int pos) = 0;
    virtual bool cmd_CPosExit_Ex(const QString &eid) = 0;
    virtual bool cmd_CPosLeft(int pos) = 0;
    virtual bool cmd_CPosRight(int pos) = 0;

    virtual void updatePosition(const QString &api) = 0;
    virtual void onFunctionFinished(const QString &){}
    void setRackCoord(int coord);

private slots:
    void mainProcess_slot();
    void cancelTimeoutTimer_slot();
    void onReviewTimeout_slot();

protected:
    bool m_isHighPriority;
    CartCoord mCoord;
    enum RelativePos {
        RePos_None,
        RePos_Single,
        RePos_Front,
        RePos_Back
    } mRelatePos;

    MExport *mE1;
    MExport *mE2;

    bool m_suspend;
    bool m_isResetting;
    bool m_hasAvoidRequest;

private:
    QSharedPointer<RtRack> mRack = nullptr;
    StationManager *mStatMgr;

    bool mLock;
    bool m_isLocked;
    bool m_isEnable;
    bool m_isAborted;
    int m_MaxWaiting;

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

        Review_Rack,
        WaitF_4,

        Recycle_Rack,
        WaitF_5,

        Move_To_Last_Postion,
        WaitF_Last_Postion,

        Reset,
        All_Finish
    } s_MainProcess;

    /* 1. Scan Barcode */
    MStation *mScanStation;
    int mScan_pos;
    QString m_scanBarCode;
    bool m_isScanRackCodeFinished;
    QString m_scanRackBarCode;
    enum class ScanState {
        Finish,

        Scan_RackCode_Idle,
        Move_To_RackScan_Pos,
        WaitF_Pos_RackScan_Done,
        Scan_RackCode,
        WaitF_Scan_RackCode_Done,

        Idle,

        Get_Scan_Station,

        Move_To_Scan_Pos,
        WaitF_Pos_Done,

        Send_Station_Scan_Request,
        WaitF_Station_Scan_Done,

        Done
    } s_Scan;
    void ScanBarcodeProcess();

    /* 2. Send Sample To Station */
    int index;
    QList<QSharedPointer<RtSample>> mUnsendSampleList;
    QSharedPointer<RtSample> mTodoSample;
    QSharedPointer<RtSample> mSendSample;
    MStation *mSendStation;
    enum class SendState {
        Finish,
        Idle,

        Get_Untest_Sample,
        Get_Matched_Station,
        WaitF_Station_Response,

        Move_To_Test_Pos,
        WaitF_Test_Pos_Done,

        Send_New_Sample_To_Station,
        WaitF_Send_Sample_To_Station_Done,

        Done
    } s_Send;
    void SendToStatProcess();

    /* 3. Receive Sample From Station */
    QSharedPointer<RtSample> mRecycleSample;
    MStation *mRecycleStation;
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

    /* 4. Review Rack if there needs smear */
    enum class ReviewRackState {
        Finish,
        Idle,

        Move_To_Review_Pos,
        WaitF_Review_Pos_Done,

        WaitF_Review_Result_Done,

        Done
    } s_ReviewRack;
    void ReviewRack();

    bool m_isReviewTimeout;
    QTimer *m_reviewTimeoutTimer;

    /* 5. Recycle Rack to Export */
    MExport *mExport;
    enum class RecycleRackState {
        Finish,
        Idle,

        Get_Export,

        Move_To_Export_Pos,
        WaitF_Export_Pos_Done,

        Req_Recycle_Rack,
        WaitF_Recycle_Rack_Done
    } s_RecycleRack;
    void RecycleRack();

    QTimer *m_timeoutTimer;
    int m_timeoutCount;

    bool isSmearOnly(const QString &code);
};

#endif // M_CART_H

