#ifndef M_EXPORT2_H
#define M_EXPORT2_H

#include "m_export.h"

class MExport2 : public MExport
{
    Q_OBJECT
public:
    explicit MExport2(QObject *parent = nullptr);

protected:
    virtual void cmd_UnloadRack() override;
    virtual bool isUnloadRack_Done() override;
    virtual void sendExitFullException() override;
    virtual void sendClearExitFullException() override;
};

#endif // M_EXPORT2_H
