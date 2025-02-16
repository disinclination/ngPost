#ifndef PATHSETTINGSWIDGET_H
#define PATHSETTINGSWIDGET_H

#include <QDialog>

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

private:
    Ui::PathSettingsWidget *ui;

protected:
    void closeEvent(QCloseEvent *event) override;
    bool _valueHasChanged;
};

#endif // PATHSETTINGSWIDGET_H
