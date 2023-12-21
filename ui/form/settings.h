#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWizardPage>

namespace Ui {
    class Settings;
    }

    class Settings : public QWizardPage
    {
        Q_OBJECT

    public:
        explicit Settings(QWidget *parent = nullptr);
        ~Settings();

    private:
        Ui::Settings *ui;
};

#endif // SETTINGS_H
