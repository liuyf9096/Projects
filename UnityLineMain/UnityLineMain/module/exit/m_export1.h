#ifndef M_EXPORT1_H
#define M_EXPORT1_H

#include "m_export.h"

class MExport1 : public MExport
{
    Q_OBJECT
public:
    explicit MExport1(QObject *parent = nullptr);

protected:
    virtual void cmd_UnloadRack() override;
    virtual bool isUnloadRack_Done() override;
    virtual void sendExitFullException() override;
    virtual void sendClearExitFullException() override;
};

#endif // M_EXPORT1_H
