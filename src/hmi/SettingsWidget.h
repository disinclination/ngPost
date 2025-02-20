#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QDialog>
#include <QMap>

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
    void OnAutoArchivePasswordToggled();

private:
    Ui::SettingsWidget *ui;
    bool hasChanges() const;
    std::string generateRandomString(int length);
    QMap<QString, QString> updatedSettings;
    void setSetting(const QString& key, const QString& value);
    

protected:
    void closeEvent(QCloseEvent *event) override;

    bool _valueHasChanged;
};

#endif // SETTINGSWIDGET_H
