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

void MCart1::cmd_Reset()
{
    dev->track2()->cmd_C1_Reset();
    MCart::cmd_Reset();
}

void MCart1::cmd_PosImport()
{
    dev->track2()->cmd_C1_Pos_Import();
    MCart::cmd_PosImport();
}

void MCart1::cmd_PosSxScan(const QString &sid, int pos)
{
    if (sid == "S1") {
        dev->track2()->cmd_C1_Pos_S1_Scan(pos);
    } else if (sid == "S2") {
        dev->track2()->cmd_C1_Pos_S2_Scan(pos);
    } else if (sid == "S3") {
        dev->track2()->cmd_C1_Pos_S3_Scan(pos);
    } else {
        return;
    }

    QString api = QString("Pos%1Scan").arg(sid);
    MCart::cmd_PosSxScan(api, pos);
}

void MCart1::cmd_PosSxTest(const QString &sid, int pos)
{
    if (sid == "S1") {
        dev->track2()->cmd_C1_Pos_S1_Test(pos);
    } else if (sid == "S2") {
        dev->track2()->cmd_C1_Pos_S2_Test(pos);
    } else if (sid == "S3") {
        dev->track2()->cmd_C1_Pos_S3_Test(pos);
    } else {
        return;
    }

    QString api = QString("Pos%1Test").arg(sid);
    MCart::cmd_PosSxTest(api, pos);
}

void MCart1::cmd_PosExit_Ex(const QString &eid)
{
    if (eid == "E1") {
        dev->track2()->cmd_C1_Pos_Exit_E1();
    } else if (eid == "E2") {
        dev->track2()->cmd_C1_Pos_Exit_E2();
    } else {
        return;
    }

    QString api = QString("PosExit_%1").arg(eid);
    MCart::cmd_PosExit_Ex(api);
}

void MCart1::cmd_PosLeft(int offset)
{
    dev->track2()->cmd_C1_Pos_Left(offset);
    MCart::cmd_PosLeft(offset);
}

void MCart1::cmd_PosRight(int offset)
{
    dev->track2()->cmd_C1_Pos_Right(offset);
    MCart::cmd_PosRight(offset);
}

bool MCart1::isCmd_Reset_done()
{
    return dev->track2()->isFuncDone("C1_Reset");
}

bool MCart1::isCmd_PosImport_done()
{
    return dev->track2()->isFuncDone("C1_Pos_Import");
}

bool MCart1::isCmd_PosScan_done(const QString &sid)
{
    QString api = QString("C1_Pos_%1_Scan").arg(sid);
    return dev->track2()->isFuncDone(api);
}

bool MCart1::isCmd_PosTest_done(const QString &sid)
{
    QString api = QString("C1_Pos_%1_Test").arg(sid);
    return dev->track2()->isFuncDone(api);
}

bool MCart1::isCmd_PosExit_Ex_done(const QString &eid)
{
    QString api = QString("C1_Pos_Exit_%1").arg(eid);
    return dev->track2()->isFuncDone(api);
}

bool MCart1::isCmd_PosLeft_done()
{
    return dev->track2()->isFuncDone("C1_Pos_Left");
}

bool MCart1::isCmd_PosRight_done()
{
    return dev->track2()->isFuncDone("C1_Pos_Right");
}

void MCart1::updatePosition(const QString &api)
{
    if (api == "C1_Reset") {
        mCoord.pos = Pos_Origin;
    } else if (api == "C1_Pos_Import") {
        mCoord.pos = Pos_Import;
    } else if (api == "C1_Pos_S1_Scan") {
        mCoord.pos = Pos_Scan_1;
    } else if (api == "C1_Pos_S1_Test") {
        mCoord.pos = Pos_Test_1;
    } else if (api == "C1_Pos_S2_Scan") {
        mCoord.pos = Pos_Scan_2;
    } else if (api == "C1_Pos_S2_Test") {
        mCoord.pos = Pos_Test_2;
    } else if (api == "C1_Pos_S3_Scan") {
        mCoord.pos = Pos_Scan_3;
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
}
