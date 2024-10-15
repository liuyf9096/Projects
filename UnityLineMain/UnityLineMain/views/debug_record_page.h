#ifndef DEBUG_RECORD_PAGE_H
#define DEBUG_RECORD_PAGE_H

#include <QWidget>
#include <QMap>

namespace Ui {
class DebugRecordPage;
}

class QPlainTextEdit;
class DebugRecordPage : public QWidget
{
    Q_OBJECT

public:
    explicit DebugRecordPage(QWidget *parent = nullptr);
    ~DebugRecordPage();

    void configTabs();
    void addTab(const QString &name);

private:
    Ui::DebugRecordPage *ui;

    QMap<QString, QPlainTextEdit *> m_deviceTextMap;

    QString getDeviceType(const QString &message);

private slots:
    void handleDebugMessage_slot(const QJsonObject &obj);
};

#endif // DEBUG_RECORD_PAGE_H
