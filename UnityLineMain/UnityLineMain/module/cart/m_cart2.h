#ifndef MCART2_H
#define MCART2_H

#include "m_cart.h"

class MCart2 : public MCart
{
    Q_OBJECT
public:
    explicit MCart2(QObject *parent = nullptr);

    virtual bool req_Reset() override;
    virtual bool req_PosImport() override;
    virtual bool req_PosSxScanRackCode(const QString &sid) override;
    virtual bool req_PosSxScan(const QString &sid, int pos) override;
    virtual bool req_PosSxTest(const QString &sid, int pos) override;
    virtual bool req_PosExitEx(const QString &eid) override;
    virtual bool req_PosLeft(int offset) override;
    virtual bool req_PosRight(int offset) override;

    virtual bool isCmd_Reset_done() override;
    virtual bool isCmd_PosImport_done() override;
    virtual bool isCmd_PosScanRackCode_done(const QString &sid) override;
    virtual bool isCmd_PosScan_done(const QString &sid) override;
    virtual bool isCmd_PosTest_done(const QString &sid) override;
    virtual bool isCmd_PosExit_Ex_done(const QString &eid) override;
    virtual bool isCmd_PosLeft_done() override;
    virtual bool isCmd_PosRight_done() override;

protected:
    virtual bool cmd_CReset() override;
    virtual bool cmd_CPosImport() override;
    virtual bool cmd_CPosScanRackCode(const QString &sid) override;
    virtual bool cmd_CPosSxScan(const QString &sid, int pos) override;
    virtual bool cmd_CPosSxTest(const QString &sid, int pos) override;
    virtual bool cmd_CPosExit_Ex(const QString &eid) override;
    virtual bool cmd_CPosLeft(int offset) override;
    virtual bool cmd_CPosRight(int offset) override;

    virtual void updatePosition(const QString &api) override;
    virtual void onFunctionFinished(const QString &api) override;
};

#endif // MCART2_H
