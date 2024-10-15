#ifndef EXPORT_MANAGER_H
#define EXPORT_MANAGER_H

#include "m_manager_base.h"
#include "m_export.h"

class ExportManager : public MManagerBase
{
    Q_OBJECT
public:
    static ExportManager *GetInstance();

    MExport *mExport1;
    MExport *mExport2;

private:
    explicit ExportManager(QObject *parent = nullptr);

};

#endif // EXPORT_MANAGER_H
