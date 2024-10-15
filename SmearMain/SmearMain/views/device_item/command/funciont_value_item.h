#ifndef FUNCIONT_VALUE_ITEM_H
#define FUNCIONT_VALUE_ITEM_H

#include <QWidget>

namespace Ui {
class FunciontValueItem;
}

enum ArgumentType {
    Arg_None,
    Arg_Integer_1,
    Arg_Integer_2,
    Arg_Integer_3,
    Arg_Integer_4,
    Arg_String,
    Arg_Int_String
};

class FunciontValueItem : public QWidget
{
    Q_OBJECT

public:
    explicit FunciontValueItem(QWidget *parent = nullptr);
    ~FunciontValueItem();

    void setApi(const QString &api);
    void setArgType(ArgumentType type);

signals:
    void onFunctionExe_signal(const QString &api, const QJsonValue &arg);

private slots:
    void on_pushButton_clicked();

private:
    Ui::FunciontValueItem *ui;
    ArgumentType m_argType;
};

#endif // FUNCIONT_VALUE_ITEM_H
