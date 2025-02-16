#ifndef PATHSETTINGSWIDGET_H
#define PATHSETTINGSWIDGET_H

#include <QDialog>
#include <QLineEdit>

namespace Ui {
class PathSettingsWidget;
}

class PathSettingsWidget : public QDialog
{
    Q_OBJECT

public:
    explicit PathSettingsWidget(QWidget *parent = nullptr);
    ~PathSettingsWidget();

private slots:
    void HandleCancel();
    void OnTempPathButtonClicked();
    void OnNzbPathButtonClicked();
    void OnRarPathButtonClicked();
    void OnPar2PathButtonClicked();
    void OnPostHistoryPathButtonClicked();
    void OnLogPathButtonClicked();
    void OnValueChanged();

private:
    Ui::PathSettingsWidget *ui;
    void selectDirectory(QLineEdit *lineEdit);
    bool hasChanges() const;

protected:
    void closeEvent(QCloseEvent *event) override;
    bool _valueHasChanged;
};

#endif // PATHSETTINGSWIDGET_H
