#include "stain_pool_page.h"
#include "ui_stain_pool_page.h"

//#include "stain/pool/pool_manager.h"
#include "pool_form.h"
#include "stain/stain_manager.h"

#include <QDebug>

StainPoolPage::StainPoolPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StainPoolPage)
{
    ui->setupUi(this);
    init();

//    connect(PoolManager::GetInstance(), &PoolManager::onStartStain_signal,
//            this, &StainPoolPage::onStartStain_slot);
//    connect(PoolManager::GetInstance(), &PoolManager::onTakeOutOfPool_signal,
//            this, &StainPoolPage::onTakeOutofPool_slot);

//    connect(MRecycleBoxMgr::GetInstance(), &MRecycleBoxMgr::onRecycleSampleFinished_signal,
//            this, &StainPoolPage::onRecycleSampleFinished_slot);
}

StainPoolPage::~StainPoolPage()
{
    delete ui;
}

void StainPoolPage::init()
{
    for (int i = 21; i < 25; ++i) {
        PoolForm *form = new PoolForm(this);
        form->setTitle(QString::number(i));
        ui->fixLayout->insertWidget(0, form);
        m_formMap.insert(i, form);
    }
    for (int i = 25; i < 32; ++i) {
        PoolForm *form = new PoolForm(this);
        form->setTitle(QString::number(i));
        ui->a1Layout->insertWidget(0, form);
        m_formMap.insert(i, form);
    }
    for (int i = 33; i < 40; ++i) {
        PoolForm *form = new PoolForm(this);
        form->setTitle(QString::number(i));
        ui->c1Layout->insertWidget(0, form);
        m_formMap.insert(i, form);
    }
    for (int i = 40; i < 41; ++i) {
        PoolForm *form = new PoolForm(this);
        form->setTitle(QString::number(i));
        ui->transferLayout->insertWidget(0, form);
        m_formMap.insert(i, form);
    }
    for (int i = 41; i < 51; ++i) {
        PoolForm *form = new PoolForm(this);
        form->setTitle(QString::number(i));
        ui->c2aLayout->insertWidget(0, form);
        m_formMap.insert(i, form);
    }
    for (int i = 51; i < 61; ++i) {
        PoolForm *form = new PoolForm(this);
        form->setTitle(QString::number(i));
        ui->c2bLayout->insertWidget(0, form);
        m_formMap.insert(i, form);
    }
    for (int i = 61; i < 63; ++i) {
        PoolForm *form = new PoolForm(this);
        form->setTitle(QString::number(i));
        ui->washLayout->insertWidget(0, form);
        m_formMap.insert(i, form);
    }
    for (int i = 63; i < 66; ++i) {
        ;
    }
    for (int i = 66; i < 76; ++i) {
        PoolForm *form = new PoolForm(this);
        form->setTitle(QString::number(i));
        ui->boxLayout->insertWidget(-1, form);
        m_formMap.insert(i, form);
    }
}

void StainPoolPage::onStartStain_slot(int pos, const QString &sid, int interval)
{
    if (m_formMap.contains(pos)) {
        auto form = m_formMap.value(pos);
        form->setSampleId(sid);
        form->start(interval);
    }
}

void StainPoolPage::onTakeOutofPool_slot(int pos, const QString &sid)
{
    Q_UNUSED(sid)

    if (m_formMap.contains(pos)) {
        auto form = m_formMap.value(pos);
        form->clear();
    }
}

void StainPoolPage::onRecycleSampleFinished_slot(int pos, const QString &sid)
{
    if (m_formMap.contains(pos)) {
        auto form = m_formMap.value(pos);
        form->setSampleId(sid);
    }
}

