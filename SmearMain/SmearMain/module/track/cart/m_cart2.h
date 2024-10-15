#ifndef M_CART2_H
#define M_CART2_H

#include "m_cart.h"

class MCart2 : public MCart
{
    Q_OBJECT
public:
    explicit MCart2(QObject *parent = nullptr);

    virtual bool req_Reset() override;
    virtual bool req_PosImport() override;
    virtual bool req_PosScan(int pos) override;
    virtual bool req_PosTest(int pos) override;
    virtual bool req_PosExport() override;
    virtual bool req_PosLeft(int offset) override;
    virtual bool req_PosRight(int offset) override;

    virtual bool isCmd_Reset_done() override;
    virtual bool isCmd_PosImport_done() override;
    virtual bool isCmd_PosScan_done() override;
    virtual bool isCmd_PosTest_done() override;
    virtual bool isCmd_PosExport_done() override;
    virtual bool isCmd_PosLeft_done() override;
    virtual bool isCmd_PosRight_done() override;

protected:
    virtual bool cmd_CReset() override;
    virtual bool cmd_CPosImport() override;
    virtual bool cmd_CPosScan(int pos) override;
    virtual bool cmd_CPosTest(int pos) override;
    virtual bool cmd_CPosExport() override;
    virtual bool cmd_CPosLeft(int pos) override;
    virtual bool cmd_CPosRight(int pos) override;

    virtual void updatePosition(const QString &api) override;
    virtual void onFunctionFinished(const QString &api) override;
};

#endif // M_CART2_H
