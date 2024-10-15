#ifndef M_EXPORT_H
#define M_EXPORT_H

#include "module_base.h"

class RtRack;
class MExport : public DModuleBase
{
    Q_OBJECT
public:
    explicit MExport(const QString &mid, QObject *parent = nullptr);

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

    bool recycleRack(QSharedPointer<RtRack> rack);
    bool isRecycleRackFinished() { return m_isRecycleFinished; }
    bool isFull();

private:
    bool m_isRecycleFinished;
    QSharedPointer<RtRack> m_recycleRack;
    QTimer *m_exportTimer;
    enum class RecycleState {
        Idle,

        Check_Full_Status,
        WaitF_UnFull,

        Send_Unload_Rack_Cmd,
        WaitF_Unload_Rack_Cmd_Done,

        Finish
    } m_state;

private slots:
    void onExportTimer_slot();
};

#endif // M_EXPORT_H
