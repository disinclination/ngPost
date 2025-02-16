#include "PathSettingsWidget.h"
#include "ui_PathSettingsWidget.h"

#include <QMessageBox>
#include <QCloseEvent>

PathSettingsWidget::PathSettingsWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PathSettingsWidget)
    , _valueHasChanged(false)
{
    ui->setupUi(this);

    // Setup the dialog
    setWindowTitle("ngPost - Path Settings");
    setWindowIcon(QIcon(":/icons/ngPost.png"));
    setFixedSize(500, 400);

    // Disable the maximize button and resizing
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    // Button related
    connect(ui->cancelButton, &QPushButton::clicked, this, &PathSettingsWidget::HandleCancel);
}

PathSettingsWidget::~PathSettingsWidget()
{
    delete ui;
}

void PathSettingsWidget::HandleCancel()
{
    if (_valueHasChanged) {
        // Create a warning message box
        QMessageBox::StandardButton reply;
        reply = QMessageBox::warning(this, "Unsaved Changes",
                                      "There are changes that have not been saved. Are you sure you want to close?",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            reject();  // Close the dialog
        }
    } else {
        reject();  // Accept the close event if no changes
    }
}

void PathSettingsWidget::closeEvent(QCloseEvent *event)
{
    // Handle the cancellation logic inside HandleCancel
    HandleCancel();

    if (_valueHasChanged) {
        event->ignore();  // Prevent the close event if there are unsaved changes
    } else {
        event->accept();  // Allow the dialog to close immediately
    }
}