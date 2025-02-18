#ifndef PATHSETTINGSWIDGET_H
#define PATHSETTINGSWIDGET_H

#include "utils/ConfigKeys.h"
#include "utils/ConfigUtility.h"

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
    void selectFile(QLineEdit *lineEdit);
    bool hasChanges() const;
    void LoadPaths();
    void updateConfigWithPath(QString configKey, QLineEdit* pathField, bool isFile = false) {
        ConfigUtility configUtility;
        
        if (isFile) {
            selectFile(pathField);
        } else {
            selectDirectory(pathField);
        }

        configUtility.UpdateField(configKey, pathField->text());
    }

protected:
    void closeEvent(QCloseEvent *event) override;
    bool _valueHasChanged;
};

#endif // PATHSETTINGSWIDGET_H
