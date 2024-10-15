#include "carts_manager.h"
#include "sql/f_sql_database_manager.h"
#include "m_cart1.h"
#include "m_cart2.h"

static const int GAP = 1;
static const int Coord_Max = 29;

#include <QTimer>
#include <QDateTime>
#include <QDebug>

//#define DebugTrackCart

CartsManager *CartsManager::GetInstance()
{
    static CartsManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new CartsManager();
    }
    return instance;
}

CartsManager::CartsManager(QObject *parent) : QObject(parent)
{
    mCart1 = new MCart1(this);
    mCart2 = new MCart2(this);

    m_state = CooperateState::Idle;
    m_avoidCart = nullptr;

    m_timer = new QTimer(this);
    m_timer->setInterval(100);
    connect(m_timer, &QTimer::timeout,
            this, &CartsManager::onCoopTimer_slot);
}

void CartsManager::start()
{
    mCart1->start();
    mCart2->start();
}

void CartsManager::reset()
{
    mCart1->reset();
    mCart2->reset();
}

void CartsManager::stop()
{
    mCart1->stop();
    mCart2->stop();
}

void CartsManager::suspend()
{
    mCart1->suspend();
    mCart2->suspend();
}

void CartsManager::resume()
{
    mCart1->resume();
    mCart2->resume();
}

void CartsManager::abort()
{
    mCart1->abort();
    mCart2->abort();
}

void CartsManager::setExport(MExport *exp)
{
    mCart1->setExport(exp);
    mCart2->setExport(exp);
}

bool CartsManager::requestMoveAccess(MCart *cart, int to)
{
    if (to < 0 || to > Coord_Max) {
        return false;
    }

    if (cart == mCart1 && mCart2->mCoord.pos == Pos_Origin) {
        mCart1->m_isHighPriority = true;
        mCart1->mCoord.setRange(to);
        return true;
    } else if (cart == mCart2 && mCart1->mCoord.pos == Pos_Origin) {
        mCart2->m_isHighPriority = true;
        mCart2->mCoord.setRange(to);
        return true;
    } else {
        if (cart == mCart1)
        {
            if (to == 0) {          /* C1 Reset */
                if (mCart1->mCoord.coord > mCart2->mCoord.coord) {  /* <C1...C2 */
                    mCart1->m_isHighPriority = false;
                    mCart2->m_isHighPriority = true;
                    return true;
                }
            } else {                /* C1 Move */
                if (mCart1->mCoord.right > mCart2->mCoord.left) {   /* <C1>...C2 */
                    if (to - mCart1->mCoord.Width > mCart2->mCoord.left + GAP) {
                        mCart1->mCoord.setRange(to);
                        mCart1->m_isHighPriority = true;
                        mCart2->m_isHighPriority = false;
                        return true;
                    } else {
                        coorerateCarts(mCart2);
                    }
                } else {                                            /* C2...<C1> */
                    if (mCart2->isResetting()) {
                        mCart1->mCoord.setRange(to);
                        mCart2->m_hasAvoidRequest = false;
                        mCart1->m_isHighPriority = true;
                        mCart2->m_isHighPriority = false;
                        return true;
                    } else if (mCart2->mCoord.right > to + GAP) {
                        mCart1->mCoord.setRange(to);
                        mCart2->m_hasAvoidRequest = false;
                        mCart1->m_isHighPriority = false;
                        mCart2->m_isHighPriority = true;
                        return true;
                    }
                    mCart2->m_hasAvoidRequest = true;
                }
            }
#ifdef DebugTrackCart
            qDebug().noquote() << QString("Cart1 Can not accept request: %1 -> %2")
                                  .arg(mCart1->mCoord.coord).arg(to);
#endif
            return false;
        }
        else if (cart == mCart2)
        {
            if (to == 0) {          /* C2 Reset */
                if (mCart2->mCoord.coord > mCart1->mCoord.coord) {   /* <C2...C1 */
                    mCart1->m_isHighPriority = true;
                    mCart2->m_isHighPriority = false;
                    return true;
                }
            } else {                /* C2 Move */
                if (mCart2->mCoord.right > mCart1->mCoord.left) {   /* <C2>...C1 */
                    if (to - mCart2->mCoord.Width > mCart1->mCoord.left + GAP) {
                        mCart2->mCoord.setRange(to);
                        mCart1->m_isHighPriority = false;
                        mCart2->m_isHighPriority = true;
                        return true;
                    } else {
                        coorerateCarts(mCart1);
                    }
                } else {                                            /* C1...<C2> */
                    if (mCart1->isResetting()) {
                        mCart2->mCoord.setRange(to);
                        mCart1->m_hasAvoidRequest = false;
                        mCart1->m_isHighPriority = false;
                        mCart2->m_isHighPriority = true;
                        return true;
                    } else if (mCart1->mCoord.right > to + GAP) {
                        mCart2->mCoord.setRange(to);
                        mCart1->m_hasAvoidRequest = false;
                        mCart1->m_isHighPriority = true;
                        mCart2->m_isHighPriority = false;
                        return true;
                    }
                    mCart1->m_hasAvoidRequest = true;
                }
            }

#ifdef DebugTrackCart
            qDebug().noquote() << QString("Cart2 Can not accept request: %1 -> %2")
                                  .arg(mCart2->mCoord.coord).arg(to);
#endif
            return false;
        }
    }

    /* error */
    qDebug().noquote() << QString("[%1] Can not accept request. Current:%2, request toCoord:%3")
                          .arg(cart->userid(), cart->mCoord.getCoorStr()).arg(to);
    return false;
}

void CartsManager::updateCartRelativePos()
{
    if (mCart1->mCoord.pos == Pos_Origin && mCart2->mCoord.pos == Pos_Origin) {
        mCart1->mRelatePos = MCart::RePos_None;
        mCart2->mRelatePos = MCart::RePos_None;
    } else if (mCart1->mCoord.pos == Pos_Origin && mCart2->mCoord.pos != Pos_Origin) {
        mCart1->mRelatePos = MCart::RePos_None;
        mCart2->mRelatePos = MCart::RePos_Single;
    } else if (mCart1->mCoord.pos != Pos_Origin && mCart2->mCoord.pos == Pos_Origin) {
        mCart1->mRelatePos = MCart::RePos_Single;
        mCart2->mRelatePos = MCart::RePos_None;
    } else if (mCart1->isResetting() && mCart2->mCoord.pos != Pos_Origin) {
        mCart1->mRelatePos = MCart::RePos_None;
        mCart2->mRelatePos = MCart::RePos_Single;
    } else if (mCart1->mCoord.pos != Pos_Origin && mCart2->isResetting()) {
        mCart1->mRelatePos = MCart::RePos_Single;
        mCart2->mRelatePos = MCart::RePos_None;
    } else if (mCart1->mCoord.coord > mCart2->mCoord.coord) {
        mCart1->mRelatePos = MCart::RePos_Front;
        mCart2->mRelatePos = MCart::RePos_Back;
    } else if (mCart1->mCoord.coord < mCart2->mCoord.coord) {
        mCart1->mRelatePos = MCart::RePos_Back;
        mCart2->mRelatePos = MCart::RePos_Front;
    }
}

MCart *CartsManager::getSpareCart(bool *ok)
{
    if (mCart1->mCoord.pos == Pos_Origin && mCart2->mCoord.pos == Pos_Origin) {
        if (mCart1->isEnable()) {
            *ok = true;
            return mCart1;
        } else if (mCart2->isEnable()) {
            *ok = true;
            return mCart2;
        }
    } else {
        if (mCart1->mCoord.pos == Pos_Origin && mCart2->permitAlliedBack()) {
            if (mCart1->isEnable()) {
                *ok = true;
                return mCart1;
            }
        } else if (mCart2->mCoord.pos == Pos_Origin && mCart1->permitAlliedBack()) {
            if (mCart2->isEnable()) {
                *ok = true;
                return mCart2;
            }
        }
    }
    *ok = false;
    return nullptr;
}

void CartsManager::coorerateCarts(MCart *avoid)
{
    if (avoid != nullptr && m_avoidCart == nullptr) {
        m_avoidCart = avoid;
        m_avoidCart->setLock(true);
        m_state = CooperateState::Idle;
        m_timer->start();
    }
}

void CartsManager::onCoopTimer_slot()
{
    switch (m_state) {
    case CooperateState::Idle:
        if (m_avoidCart != nullptr) {
            logProcess("Cooperate", 0, "Lock cart", QString("cart:%1").arg(m_avoidCart->mid()));
            m_state = CooperateState::WaitF_Cart_Locked;
        }
        break;

    case CooperateState::WaitF_Cart_Locked:
        if (m_avoidCart->isLocked() == true) {
            logProcess("Cooperate", 1, "cart locked");
            m_state = CooperateState::Move_To_Safety_Pos;
        }
        break;

    case CooperateState::Move_To_Safety_Pos:
    {
        bool ok = m_avoidCart->req_PosImport();
        if (ok) {
            logProcess("Cooperate", 2, "avoid cart");
            m_state = CooperateState::WaitF_To_Safety_Pos_Done;
        }
        break;
    }
    case CooperateState::WaitF_To_Safety_Pos_Done:
        if (m_avoidCart->isCmd_PosImport_done() == true) {
            m_state = CooperateState::Unlock_Cart;
        }
        break;

    case CooperateState::Unlock_Cart:
        logProcess("Cooperate", 3, "unlock cart");
        m_avoidCart->setLock(false);
        m_state = CooperateState::Finish;
        break;

    case CooperateState::Finish:
        logProcess("Cooperate", 99, "Finish");
        m_avoidCart = nullptr;
        m_timer->stop();
        m_state = CooperateState::Idle;
        break;
    }
}

void CartsManager::logProcess(const QString &process, int state, const QString &statemsg, const QString &message)
{
    QJsonObject obj;
    obj.insert("time", QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz"));

    QString str;
    if (state >= 0) {
        str = QString("{%1}(%2 %3)").arg(process).arg(state).arg(statemsg);
    } else {
        str = QString("{%1} %2").arg(process, statemsg);
    }

    if (!message.isEmpty()) {
        str.append(message);
    }
    obj.insert("cartsmanager", str);

    auto logDb = FSqlDatabaseManager::GetInstance()->getDatebase("log");
    if (logDb) {
        logDb->insertRecord("track", obj);
    }
}


