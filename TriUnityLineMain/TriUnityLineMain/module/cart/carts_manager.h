#ifndef CARTS_MANAGER_H
#define CARTS_MANAGER_H

#include "m_manager_base.h"
#include "m_cart.h"

class QTimer;
class CartsManager : public MManagerBase
{
    Q_OBJECT
public:
    static CartsManager *GetInstance();

    MCart *mCart1;
    MCart *mCart2;

    void setExport1(MExport *exp);
    void setExport2(MExport *exp);

    MCart *getSpareCart();

    bool requestMoveAccess(MCart *cart, int to);

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
