#include "SettingsWidget.h"
#include "ui_SettingsWidget.h"

#include <QMessageBox>
#include <QCloseEvent>

SettingsWidget::SettingsWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsWidget)
    , _valueHasChanged(false)
{
    ui->setupUi(this);

    // Setup the dialog
    setWindowTitle("ngPost - Settings");
    setWindowIcon(QIcon(":/icons/ngPost.png"));
    setFixedSize(500, 500);

    // Disable the maximize button and resizing
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    ui->saveButton->setEnabled(false);

    connect(ui->anonymousProxyCheckBox, &QCheckBox::toggled, this, &SettingsWidget::OnAnonymousProxyToggled);

    // Catch any changes on the UI
    // Proxy related
    connect(ui->anonymousProxyCheckBox, &QCheckBox::toggled, this, &SettingsWidget::onValueChanged);
    connect(ui->proxyUsernameTextField, &QLineEdit::textChanged, this, &SettingsWidget::onValueChanged);
    connect(ui->proxyPasswordTextField,&QLineEdit::textChanged, this, &SettingsWidget::onValueChanged);
    connect(ui->proxyHostnameTextField, &QLineEdit::textChanged, this, &SettingsWidget::onValueChanged);
    connect(ui->proxyHttpRadioButton, &QRadioButton::toggled, this, &SettingsWidget::onValueChanged);
    connect(ui->proxyHttpsRadioButton, &QRadioButton::toggled, this, &SettingsWidget::onValueChanged);
    connect(ui->proxySocks4RadioButton, &QRadioButton::toggled, this, &SettingsWidget::onValueChanged);
    connect(ui->proxySocks5RadioButton, &QRadioButton::toggled, this, &SettingsWidget::onValueChanged);

    // Button related
    connect(ui->cancelButton, &QPushButton::clicked, this, &SettingsWidget::handleCancel);


    // Make the dialog modal
    setModal(true);
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}

bool SettingsWidget::hasChanges() const
{
    return _valueHasChanged;
}

void SettingsWidget::onValueChanged()
{
    _valueHasChanged = true;

    ui->saveButton->setEnabled(true);
}

void SettingsWidget::handleCancel()
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

void SettingsWidget::closeEvent(QCloseEvent *event)
{
    // Handle the cancellation logic inside handleCancel
    handleCancel();

    if (_valueHasChanged) {
        event->ignore();  // Prevent the close event if there are unsaved changes
    } else {
        event->accept();  // Allow the dialog to close immediately
    }
}

void SettingsWidget::OnAnonymousProxyToggled(){
    qDebug() << "Anonymous proxy toggled";
    ui->proxyUsernameTextField->setDisabled(ui->anonymousProxyCheckBox->isChecked());
    ui->proxyPasswordTextField->setDisabled(ui->anonymousProxyCheckBox->isChecked());

    _valueHasChanged = true;
}

