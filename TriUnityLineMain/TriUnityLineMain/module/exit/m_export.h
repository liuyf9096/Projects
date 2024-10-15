#ifndef M_EXPORT_H
#define M_EXPORT_H

#include "module_base.h"

class RtRack;
class MExport : public DModuleBase
{
    Q_OBJECT
public:
    explicit MExport(const QString &mid, const QString &userid, QObject *parent = nullptr);

    void setEnable(bool en) { m_isEnable = en; }
    bool isEnable() { return m_isEnable; }

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

    bool recycleRack(QSharedPointer<RtRack> rack);
    bool isRecycleRackFinished() { return m_isRecycleFinished; }
    bool isFull();

protected:
    virtual void cmd_UnloadRack() = 0;
    virtual bool isUnloadRack_Done() = 0;

private:
    bool m_isEnable;
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
