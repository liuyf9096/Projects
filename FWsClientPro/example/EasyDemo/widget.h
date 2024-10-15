#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

class FWsClient;
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void connectServer(const QString &ip, quint16 port);
    void sendTestMessage();

private:
    FWsClient *m_client;
};
#endif // WIDGET_H
