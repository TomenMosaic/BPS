#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
}

Settings::~Settings()
{
    delete ui;
}
