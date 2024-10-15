#ifndef M_SLIDESTORE_H
#define M_SLIDESTORE_H

#include "module_base.h"

class MSmear;
class MSlideStore : public DModuleBase
{
    Q_OBJECT
public:
    explicit MSlideStore(const QString &mid, QObject *parent = nullptr);
    void setSmear(MSmear *smear) { mSmear = smear; }

    void addNewSlideRequest(QSharedPointer<RtSlide> slide);
    void addPrintOnlyRequest(quint64 id, const QJsonArray &arr);
    void cancelPrintOnlyRequest();
    void setPrintMode(const QJsonObject &obj);

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

private:
    MSmear *mSmear = nullptr;

    bool m_isPrintEn;
    bool m_isScanSlideCode;
    QString m_slideQRCode;

    enum StoreIndex {
        Store_None,
        Store_1 = 1,
        Store_2 = 2
    } mStoreIndex;

    bool cmd_NewSlide(StoreIndex index, bool print);

    int m_printMode;
    QTimer *mTimer;
    QString m_api;
    bool m_isNoSlideStore;

    void state_init();

    QList<QSharedPointer<RtSlide>> mPrintOnlyList;
    QList<QSharedPointer<RtSlide>> mRequestList;
    QSharedPointer<RtSlide> m_slide;

    enum class SlideState {
        Idle,
        Get_Request,

        Check_Slide_Store,
        WaitF_Slide_Store,

        New_Slide,
        WaitF_New_Slide_Done,

        SettingBarcode,
        SettingRow1,
        SettingRow2,
        SettingRow3,

        Print_Slide,
        WaitF_Print_Done,

        Print_Head_Down,
        Print_Test_Info,
        Print_Test_Info1,
        Print_QRCode,
        Print_QRCode1,
        Print_Head_Up,
        WaitF_Print_Done1,

        Scan_Slide,
        WaitF_Scan_Done,

        Distribute_Slide,

        Move_To_Add_Sample_Pos,
        WaitF_Add_Sample_Pos_Done,

        WaitF_Add_Blood,

        Move_To_Stain_Cart_Pos,
        WaitF_Stain_Cart_Pos_Done,

        Finish,
        Error
    } s_state;

    QList<StoreIndex> getAvailableIndex();
    StoreIndex getStoreIndex();

    QString getPrintInfo(QSharedPointer<RtSlide> slide, int row);
    void convertPrintInfo(QSharedPointer<RtSlide> slide);
    QString testQRcode();

private slots:
    void onStateTimer_slot();
    void onFunctionFinished_slot(const QString &api, const QJsonValue &resValue);
};

#endif // M_SLIDESTORE_H
