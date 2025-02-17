#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QDialog>

namespace Ui {
class SettingsWidget;
}

class SettingsWidget : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = nullptr);
    ~SettingsWidget();

private slots:
    void OnAnonymousProxyToggled();
    void OnValueChanged();
    void HandleCancel();
    void OnPathSettingsClicked();
    void OnPasswordGenerateClicked();

private:
    Ui::SettingsWidget *ui;
    bool hasChanges() const;
    std::string generateRandomString(int length);

protected:
    void closeEvent(QCloseEvent *event) override;

    bool _valueHasChanged;
};

#endif // SETTINGSWIDGET_H
