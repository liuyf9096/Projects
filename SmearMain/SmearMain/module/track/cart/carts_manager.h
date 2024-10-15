#ifndef CARTS_MANAGER_H
#define CARTS_MANAGER_H

#include <QObject>
#include <QMap>
#include "m_cart.h"

class QTimer;
class MExport;
class CartsManager : public QObject
{
    Q_OBJECT
public:
    static CartsManager *GetInstance();

    void start();
    void reset();
    void stop();
    void suspend();
    void resume();
    void abort();

    MCart *mCart1;
    MCart *mCart2;

    void setExport(MExport *exp);

    MCart *getSpareCart(bool *ok);

    bool requestMoveAccess(MCart *cart, int to);
    void updateCartRelativePos();

private:
    explicit CartsManager(QObject *parent = nullptr);
    Q_DISABLE_COPY(CartsManager)

    void coorerateCarts(MCart *avoid);

    MCart *m_avoidCart;

    QTimer *m_timer;
    enum class CooperateState {
        Idle,
        WaitF_Cart_Locked,

        Move_To_Safety_Pos,
        WaitF_To_Safety_Pos_Done,

        Unlock_Cart,
        Finish
    } m_state;

    void logProcess(const QString &process, int state, const QString &statemsg, const QString &message = QString());

private slots:
    void onCoopTimer_slot();
};

#endif // CARTS_MANAGER_H
