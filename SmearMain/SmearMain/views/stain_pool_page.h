#ifndef STAIN_POOL_PAGE_H
#define STAIN_POOL_PAGE_H

#include <QWidget>
#include <QMap>

namespace Ui {
class StainPoolPage;
}

class PoolForm;
class StainPoolPage : public QWidget
{
    Q_OBJECT

public:
    explicit StainPoolPage(QWidget *parent = nullptr);
    ~StainPoolPage();

private slots:
    void onStartStain_slot(int pos, const QString &sid, int interval);
    void onTakeOutofPool_slot(int pos, const QString &sid);
    void onRecycleSampleFinished_slot(int pos, const QString &sid);

private:
    Ui::StainPoolPage *ui;

    void init();
    QMap<int, PoolForm *> m_formMap;
};

#endif // STAIN_POOL_PAGE_H
