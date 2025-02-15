#include "SettingsWidget.h"
#include "ui_SettingsWidget.h"

SettingsWidget::SettingsWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);

    // Setup the dialog
    setWindowTitle("ngPost - Settings");
    setWindowIcon(QIcon(":/icons/ngPost.png"));
    setFixedSize(500, 500);

    // Disable the maximize button and resizing
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    // Make the dialog modal
    setModal(true);
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}
