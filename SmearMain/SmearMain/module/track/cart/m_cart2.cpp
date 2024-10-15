#include "m_cart2.h"
#include "carts_manager.h"

MCart2::MCart2(QObject *parent)
    : MCart{"cart2", "C2", parent}
{
    
}

bool MCart2::req_Reset()
{
    int to = CartCoord::CoordBaseMap.value("Reset");
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_Reset();
    }
    return ok;
}

bool MCart2::req_PosImport()
{
    int to = CartCoord::CoordBaseMap.value("PosImport");
    Q_ASSERT(to > 0);
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosImport();
    }
    return ok;
}

bool MCart2::req_PosScan(int pos)
{
    int to = CartCoord::CoordBaseMap.value("PosScan") + pos;
    Q_ASSERT(to > pos);
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosScan(pos);
    }
    return ok;
}

bool MCart2::req_PosTest(int pos)
{
    int to = CartCoord::CoordBaseMap.value("PosTest") + pos;
    Q_ASSERT(to > pos);
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosTest(pos);
    }
    return ok;
}

bool MCart2::req_PosExport()
{
    int to = CartCoord::CoordBaseMap.value("PosExport");
    Q_ASSERT(to > 0);
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosExport();
    }
    return ok;
}

bool MCart2::req_PosLeft(int offset)
{
    int to = mCoord.coord + offset;
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosLeft(offset);
    }
    return ok;
}

bool MCart2::req_PosRight(int offset)
{
    int to = mCoord.coord - offset;
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosRight(offset);
    }
    return ok;
}

bool MCart2::cmd_CReset()
{
    return dev->track()->cmd_C2_Reset();
}

bool MCart2::cmd_CPosImport()
{
    return dev->track()->cmd_C2_Pos_Import();
}

bool MCart2::cmd_CPosScan(int pos)
{
    return dev->track()->cmd_C2_Pos_Scan(pos);
}

bool MCart2::cmd_CPosTest(int pos)
{
    return dev->track()->cmd_C2_Pos_Test(pos);
}

bool MCart2::cmd_CPosExport()
{
    return dev->track()->cmd_C2_Pos_Export();
}

bool MCart2::cmd_CPosLeft(int pos)
{
    return dev->track()->cmd_C2_Pos_Left(pos);
}

bool MCart2::cmd_CPosRight(int pos)
{
    return dev->track()->cmd_C2_Pos_Right(pos);
}

bool MCart2::isCmd_Reset_done()
{
    return dev->track()->isFuncDone("C2_Reset");
}

bool MCart2::isCmd_PosImport_done()
{
    return dev->track()->isFuncDone("C2_Pos_Import");
}

bool MCart2::isCmd_PosScan_done()
{
    return dev->track()->isFuncDone("C2_Pos_Scan");
}

bool MCart2::isCmd_PosTest_done()
{
    return dev->track()->isFuncDone("C2_Pos_Test");
}

bool MCart2::isCmd_PosExport_done()
{
    return dev->track()->isFuncDone("C2_Pos_Export");
}

bool MCart2::isCmd_PosLeft_done()
{
    return dev->track()->isFuncDone("C2_Pos_Left");
}

bool MCart2::isCmd_PosRight_done()
{
    return dev->track()->isFuncDone("C2_Pos_Right");
}

void MCart2::updatePosition(const QString &api)
{
    if (api == "C2_Reset") {
        mCoord.pos = Pos_Origin;
    } else if (api == "C2_Pos_Import") {
        mCoord.pos = Pos_Import;
    } else if (api == "C2_Pos_Scan") {
        mCoord.pos = Pos_Scan;
    } else if (api == "C2_Pos_Test") {
        mCoord.pos = Pos_Test;
    } else if (api == "C2_Pos_Export") {
        mCoord.pos = Pos_Export;
    } else {
        qDebug() << "can NOT handle Cart1 updatePosition api:" << api;
        return;
    }
    mCoord.setCoord(mCoord.toCoord);
    CartsManager::GetInstance()->updateCartRelativePos();
}

void MCart2::onFunctionFinished(const QString &api)
{
    if (api == "C2_Reset") {
        m_isResetting = false;
        mRelatePos = MCart::RePos_None;
    }
}
