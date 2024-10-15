#ifndef FUNCIONT_VALUE_ITEM_H
#define FUNCIONT_VALUE_ITEM_H

#include <QWidget>

namespace Ui {
class FunciontValueItem;
}

class FunciontValueItem : public QWidget
{
    Q_OBJECT

public:
    explicit FunciontValueItem(QWidget *parent = nullptr);
    ~FunciontValueItem();

    void setApi(const QString &api);
    void setArgCount(int count);

signals:
    void onFunctionClicked_signal(const QString &api, int argCount, int arg1 = 0, int arg2 = 0, int arg3 = 0);

private slots:
    void on_pushButton_clicked();

private:
    Ui::FunciontValueItem *ui;
    int m_argCount;
};

#endif // FUNCIONT_VALUE_ITEM_H
