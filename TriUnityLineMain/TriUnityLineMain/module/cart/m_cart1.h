#ifndef M_CART1_H
#define M_CART1_H

#include "m_cart.h"

class MCart1 : public MCart
{
    Q_OBJECT
public:
    explicit MCart1(QObject *parent = nullptr);

    virtual bool req_Reset() override;
    virtual bool req_PosImport() override;
    virtual bool req_PosSxScan(const QString &sid, int pos) override;
    virtual bool req_PosSxTest(const QString &sid, int pos) override;
    virtual bool req_PosExitEx(const QString &eid) override;
    virtual bool req_PosLeft(int offset) override;
    virtual bool req_PosRight(int offset) override;

    virtual void cmd_Reset() override;
    virtual void cmd_PosImport() override;
    virtual void cmd_PosSxScan(const QString &sid, int pos) override;
    virtual void cmd_PosSxTest(const QString &sid, int pos) override;
    virtual void cmd_PosExit_Ex(const QString &eid) override;
    virtual void cmd_PosLeft(int offset) override;
    virtual void cmd_PosRight(int offset) override;

    virtual bool isCmd_Reset_done() override;
    virtual bool isCmd_PosImport_done() override;
    virtual bool isCmd_PosScan_done(const QString &sid) override;
    virtual bool isCmd_PosTest_done(const QString &sid) override;
    virtual bool isCmd_PosExit_Ex_done(const QString &eid) override;
    virtual bool isCmd_PosLeft_done() override;
    virtual bool isCmd_PosRight_done() override;

protected:
    virtual void updatePosition(const QString &api) override;
};

#endif // M_CART1_H
