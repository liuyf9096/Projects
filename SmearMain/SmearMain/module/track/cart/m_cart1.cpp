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

bool MCart1::req_PosScan(int pos)
{
    int to = CartCoord::CoordBaseMap.value("PosScan") + pos;
    Q_ASSERT(to > pos);
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosScan(pos);
    }
    return ok;
}

bool MCart1::req_PosTest(int pos)
{
    int to = CartCoord::CoordBaseMap.value("PosTest") + pos;
    Q_ASSERT(to > pos);
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosTest(pos);
    }
    return ok;
}

bool MCart1::req_PosExport()
{
    int to = CartCoord::CoordBaseMap.value("PosExport");
    Q_ASSERT(to > 0);
    bool ok = CartsManager::GetInstance()->requestMoveAccess(this, to);
    if (ok) {
        cmd_PosExport();
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

bool MCart1::cmd_CPosScan(int pos)
{
    return dev->track()->cmd_C1_Pos_Scan(pos);
}

bool MCart1::cmd_CPosTest(int pos)
{
    return dev->track()->cmd_C1_Pos_Test(pos);
}

bool MCart1::cmd_CPosExport()
{
    return dev->track()->cmd_C1_Pos_Export();
}

bool MCart1::cmd_CPosLeft(int pos)
{
    return dev->track()->cmd_C1_Pos_Left(pos);
}

bool MCart1::cmd_CPosRight(int pos)
{
    return dev->track()->cmd_C1_Pos_Right(pos);
}

bool MCart1::isCmd_Reset_done()
{
    return dev->track()->isFuncDone("C1_Reset");
}

bool MCart1::isCmd_PosImport_done()
{
    return dev->track()->isFuncDone("C1_Pos_Import");
}

bool MCart1::isCmd_PosScan_done()
{
    return dev->track()->isFuncDone("C1_Pos_Scan");
}

bool MCart1::isCmd_PosTest_done()
{
    return dev->track()->isFuncDone("C1_Pos_Test");
}

bool MCart1::isCmd_PosExport_done()
{
    return dev->track()->isFuncDone("C1_Pos_Export");
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
    } else if (api == "C1_Pos_Scan") {
        mCoord.pos = Pos_Scan;
    } else if (api == "C1_Pos_Test") {
        mCoord.pos = Pos_Test;
    } else if (api == "C1_Pos_Export") {
        mCoord.pos = Pos_Export;
    } else {
        qDebug() << "can NOT handle Cart1 updatePosition api:" << api;
        return;
    }
    mCoord.setCoord(mCoord.toCoord);
    CartsManager::GetInstance()->updateCartRelativePos();
}

void MCart1::onFunctionFinished(const QString &api)
{
    if (api == "C1_Reset") {
        m_isResetting = false;
        mRelatePos = MCart::RePos_None;
    }
}

