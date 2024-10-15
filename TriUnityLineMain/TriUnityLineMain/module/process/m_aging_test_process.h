#ifndef M_AGING_TEST_PROCESS_H
#define M_AGING_TEST_PROCESS_H

#include "module_base.h"

class FSettings;
class MCart;
class MAgingTestProcess : public DModuleBase
{
    Q_OBJECT
public:
    explicit MAgingTestProcess(QObject *parent = nullptr);

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

private:
    void state_init();

    FSettings *settings;
    MCart *mCart;

    int m_index;
    quint64 m_count;
    int m_exitIndex;

    QTimer *m_agingTimer;
    enum class TrackState {
        Idle,

        Check_New_Rack,
        WaitF_Check_Done,

        Move_To_Import,
        WaitF_Import_Pos_Done,

        Load_Rack,
        WaitF_Load_Rack_Done,

        Move_To_Scan1_Pos,
        WaitF_Scan1_Pos_Done,

        Open_Scan1,
        WaitF_Open_Scan1_Done,

        Move_To_Test1_Pos,
        WaitF_Test1_Done,

        Move_To_Scan2_Pos,
        WaitF_Scan2_Pos_Done,

        Open_Scan2,
        WaitF_Open_Scan2_Done,

        Move_To_Test2_Pos,
        WaitF_Test2_Done,

        Move_To_Scan3_Pos,
        WaitF_Scan3_Pos_Done,

        Open_Scan3,
        WaitF_Open_Scan3_Done,

        Move_To_Test3_Pos,
        WaitF_Test3_Done,

        Move_To_Export_Pos,
        WaitF_Export_Done,

        UnLoad_Rack,
        WaitF_UnLoad_Rack_Done,

        Reset_Cart,
        WaitF_Reset_Cart_Done,

        Reset_Carts,
        WaitF_Carts_Done,

        Finish
    } s_aging;

    void trackProcess();

    quint64 m_closet1_count;
    quint64 m_closet2_count;
    enum class ClosetState {
        Idle,

        Open_Closet,
        WaitF_Closet_Open_Done,

        Close_Closet,
        WaitF_Closet_Close_Done,

        Finish
    } s_closet1, s_closet2;

    void closet1Process();
    void closet2Process();

private slots:
    void onAgingTimer_slot();
};

#endif // M_AGING_TEST_PROCESS_H
