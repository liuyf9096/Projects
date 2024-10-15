#include "m_cart1.h"
#include "carts_manager.h"

MCart1::MCart1(QObject *parent)
    : MCart{"cart1", "C1", parent}
{

}

bool MCart1::req_Reset()
{
    int to = CartCoord::CoordBaseMap.value("Reset");
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_Reset();
    }
    return ok;
}

bool MCart1::req_PosImport()
{
    int to = CartCoord::CoordBaseMap.value("PosImport");
    Q_ASSERT(to > 0);
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosImport();
    }
    return ok;
}

bool MCart1::req_PosSxScanRackCode(const QString &sid)
{
    QString pos_s = QString("Pos%1ScanRack").arg(sid);
    int to = CartCoord::CoordBaseMap.value(pos_s);
    Q_ASSERT(to > 0);
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosSxScanRackCode(sid);
    }
    return ok;
}

bool MCart1::req_PosSxScan(const QString &sid, int pos)
{
    QString pos_s = QString("Pos%1Scan").arg(sid);
    int to = CartCoord::CoordBaseMap.value(pos_s) + pos;
    Q_ASSERT(to > pos);
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosSxScan(sid, pos);
    }
    return ok;
}

bool MCart1::req_PosSxTest(const QString &sid, int pos)
{
    QString pos_s = QString("Pos%1Test").arg(sid);
    int to = CartCoord::CoordBaseMap.value(pos_s) + pos;
    Q_ASSERT(to > pos);
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosSxTest(sid, pos);
    }
    return ok;
}

bool MCart1::req_PosExitEx(const QString &eid)
{
    QString pos_s = QString("PosExit_%1").arg(eid);
    int to = CartCoord::CoordBaseMap.value(pos_s);
    Q_ASSERT(to > 0);
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosExit_Ex(eid);
    }
    return ok;
}

bool MCart1::req_PosLeft(int offset)
{
    int to = mCoord.coord + offset;
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosLeft(offset);
    }
    return ok;
}

bool MCart1::req_PosRight(int offset)
{
    int to = mCoord.coord - offset;
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosRight(offset);
    }
    return ok;
}

bool MCart1::cmd_CReset()
{
    return dev->track()->cmd_C1_Reset();
}

bool MCart1::cmd_CPosImport()
{
    return dev->track()->cmd_C1_Pos_Import();
}

bool MCart1::cmd_CPosScanRackCode(const QString &sid)
{
    if (sid.contains("S1")) {
        return dev->track()->cmd_C1_Pos_S1_RackScan();
    } else if (sid.contains("S2")) {
        return dev->track()->cmd_C1_Pos_S2_RackScan();
    } else if (sid.contains("S3")) {
        return dev->track()->cmd_C1_Pos_S3_RackScan();
    }
    return false;
}

bool MCart1::cmd_CPosSxScan(const QString &sid, int pos)
{
    if (sid.contains("S1")) {
        return dev->track()->cmd_C1_Pos_S1_Scan(pos);
    } else if (sid.contains("S2")) {
        return dev->track()->cmd_C1_Pos_S2_Scan(pos);
    } else if (sid.contains("S3")) {
        return dev->track()->cmd_C1_Pos_S3_Scan(pos);
    }
    return false;
}

bool MCart1::cmd_CPosSxTest(const QString &sid, int pos)
{
    if (sid.contains("S1")) {
        return dev->track()->cmd_C1_Pos_S1_Test(pos);
    } else if (sid.contains("S2")) {
        return dev->track()->cmd_C1_Pos_S2_Test(pos);
    } else if (sid.contains("S3")) {
        return dev->track()->cmd_C1_Pos_S3_Test(pos);
    } else {
        return false;
    }
}

bool MCart1::cmd_CPosExit_Ex(const QString &eid)
{
    if (eid.contains("E1")) {
        return dev->track()->cmd_C1_Pos_Exit_E1();
    } else if (eid.contains("E2")) {
        return dev->track()->cmd_C1_Pos_Exit_E2();
    } else {
        return false;
    }
}

bool MCart1::cmd_CPosLeft(int offset)
{
    return dev->track()->cmd_C1_Pos_Left(offset);
}

bool MCart1::cmd_CPosRight(int offset)
{
    return dev->track()->cmd_C1_Pos_Right(offset);
}

bool MCart1::isCmd_Reset_done()
{
    return dev->track()->isFuncDone("C1_Reset");
}

bool MCart1::isCmd_PosImport_done()
{
    return dev->track()->isFuncDone("C1_Pos_Import");
}

bool MCart1::isCmd_PosScanRackCode_done(const QString &sid)
{
    QString api = QString("C1_Pos_%1_RackScan").arg(sid);
    return dev->track()->isFuncDone(api);
}

bool MCart1::isCmd_PosScan_done(const QString &sid)
{
    QString api = QString("C1_Pos_%1_Scan").arg(sid);
    return dev->track()->isFuncDone(api);
}

bool MCart1::isCmd_PosTest_done(const QString &sid)
{
    QString api = QString("C1_Pos_%1_Test").arg(sid);
    return dev->track()->isFuncDone(api);
}

bool MCart1::isCmd_PosExit_Ex_done(const QString &eid)
{
    QString api = QString("C1_Pos_Exit_%1").arg(eid);
    return dev->track()->isFuncDone(api);
}

bool MCart1::isCmd_PosLeft_done()
{
    return dev->track()->isFuncDone("C1_Pos_Left");
}

bool MCart1::isCmd_PosRight_done()
{
    return dev->track()->isFuncDone("C1_Pos_Right");
}

void MCart1::updatePosition(const QString &api)
{
    if (api == "C1_Reset") {
        mCoord.pos = Pos_Origin;
    } else if (api == "C1_Pos_Import") {
        mCoord.pos = Pos_Import;
    } else if (api == "C1_Pos_S1_Scan") {
        mCoord.pos = Pos_Scan_1;
    } else if (api == "C1_Pos_S1_RackScan") {
        mCoord.pos = Pos_Scan_RackCode_1;
    } else if (api == "C1_Pos_S1_Test") {
        mCoord.pos = Pos_Test_1;
    } else if (api == "C1_Pos_S2_Scan") {
        mCoord.pos = Pos_Scan_2;
    } else if (api == "C1_Pos_S2_RackScan") {
        mCoord.pos = Pos_Scan_RackCode_2;
    } else if (api == "C1_Pos_S2_Test") {
        mCoord.pos = Pos_Test_2;
    } else if (api == "C1_Pos_S3_Scan") {
        mCoord.pos = Pos_Scan_3;
    } else if (api == "C1_Pos_S3_RackScan") {
        mCoord.pos = Pos_Scan_RackCode_3;
    } else if (api == "C1_Pos_S3_Test") {
        mCoord.pos = Pos_Test_3;
    } else if (api == "C1_Pos_Exit_E1") {
        mCoord.pos = Pos_Exit_E1;
    } else if (api == "C1_Pos_Exit_E2") {
        mCoord.pos = Pos_Exit_E2;
    } else {
        qDebug() << "can NOT handle Cart1 updatePosition api:" << api;
        return;
    }
    mCoord.setCoord(mCoord.toCoord);
    CartsManager::GetInstance()->updateCartRelativePos();
    setRackCoord(mCoord.coord);
}

void MCart1::onFunctionFinished(const QString &api)
{
    if (api == "C1_Reset") {
        m_isResetting = false;
        mRelatePos = MCart::RePos_None;
    }
}
