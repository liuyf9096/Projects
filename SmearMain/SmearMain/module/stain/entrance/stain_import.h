#ifndef STAIN_IMPORT_H
#define STAIN_IMPORT_H

#include "module_base.h"

class StainImport : public DModuleBase
{
    Q_OBJECT
public:
    explicit StainImport(const QString &mid, int pos, QObject *parent = nullptr);

    bool isEmpty();

    bool addNewSlide(QSharedPointer<RtSlide> slide);

    bool hasRequest();
    void sendRequest();

    bool takeOutSample(const QString &sid, int pos);

signals:
    void onSendRequest_singal(const QString &from_groupid, int from_pos, const QString &sid);
    void onTakeOutOfPos_signal(int pos, const QString &sid);

private:
    int mPos;

    QSharedPointer<RtSlide> m_slide = nullptr;
};

#endif // STAIN_IMPORT_H
