#include "SettingsWidget.h"
#include "ui_SettingsWidget.h"
#include "PathSettingsWidget.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <random>

SettingsWidget::SettingsWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsWidget)
    , _valueHasChanged(false)
{
    ui->setupUi(this);

    // Setup the dialog
    setWindowTitle("ngPost - Settings");
    setWindowIcon(QIcon(":/icons/ngPost.png"));
    setFixedSize(325, 275);

    // Disable the maximize button and resizing
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    ui->saveButton->setEnabled(false);

    connect(ui->anonymousProxyCheckBox, &QCheckBox::toggled, this, &SettingsWidget::OnAnonymousProxyToggled);

    // Catch any changes on the UI
    // Proxy related
    connect(ui->anonymousProxyCheckBox, &QCheckBox::toggled, this, &SettingsWidget::OnValueChanged);
    connect(ui->proxyUsernameTextField, &QLineEdit::textChanged, this, &SettingsWidget::OnValueChanged);
    connect(ui->proxyPasswordTextField,&QLineEdit::textChanged, this, &SettingsWidget::OnValueChanged);
    connect(ui->proxyHostnameTextField, &QLineEdit::textChanged, this, &SettingsWidget::OnValueChanged);
    connect(ui->proxyHttpRadioButton, &QRadioButton::toggled, this, &SettingsWidget::OnValueChanged);
    connect(ui->proxyHttpsRadioButton, &QRadioButton::toggled, this, &SettingsWidget::OnValueChanged);
    connect(ui->proxySocks4RadioButton, &QRadioButton::toggled, this, &SettingsWidget::OnValueChanged);
    connect(ui->proxySocks5RadioButton, &QRadioButton::toggled, this, &SettingsWidget::OnValueChanged);
    connect(ui->autoGenerateArchivePasswordCheckBox, &QCheckBox::toggled, this, &SettingsWidget::OnValueChanged);

    // Button related
    connect(ui->cancelButton, &QPushButton::clicked, this, &SettingsWidget::HandleCancel);
    connect(ui->pathSettingsButton, &QPushButton::clicked, this, &SettingsWidget::OnPathSettingsClicked);
    connect(ui->generateRandomArchivePasswordButton, &QPushButton::clicked, this, &SettingsWidget::OnPasswordGenerateClicked);

    // Archive related
    connect(ui->archivePasswordTextField, &QLineEdit::textChanged, this, &SettingsWidget::OnValueChanged);
    connect(ui->autoGenerateArchivePasswordCheckBox, &QCheckBox::toggled, this, &SettingsWidget::OnAutoArchivePasswordToggled);

    ui->generateRandomArchivePasswordButton->setIcon(QIcon(":/icons/reset-password.png"));

    // Make the dialog modal
    setModal(true);
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}

void SettingsWidget::setSetting(const QString& key, const QString& value) {
    updatedSettings[key] = value;
}

bool SettingsWidget::hasChanges() const
{
    return _valueHasChanged;
}

void SettingsWidget::OnValueChanged()
{
    _valueHasChanged = true;

    ui->saveButton->setEnabled(true);
}

void SettingsWidget::HandleCancel()
{
    if (_valueHasChanged) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::warning(this, "Unsaved Changes",
                                      "There are changes that have not been saved. Are you sure you want to close?",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            reject();
        }
    } else {
        reject();
    }
}

void SettingsWidget::closeEvent(QCloseEvent *event)
{
    HandleCancel();

    if (_valueHasChanged) {
        event->ignore();
    } else {
        event->accept();
    }
}

void SettingsWidget::OnPathSettingsClicked()
{
  PathSettingsWidget pathSettingsWidget;
  pathSettingsWidget.exec();
}

void SettingsWidget::OnAnonymousProxyToggled()
{
    qDebug() << "Anonymous proxy toggled";
    ui->proxyUsernameTextField->setDisabled(ui->anonymousProxyCheckBox->isChecked());
    ui->proxyPasswordTextField->setDisabled(ui->anonymousProxyCheckBox->isChecked());

    _valueHasChanged = true;
}

void SettingsWidget::OnPasswordGenerateClicked()
{
    qDebug() << "Password generation initiated";

    auto passwordLength = ui->passwordLengthSpinBox->value();
    auto randomPassword = generateRandomString(passwordLength);

    ui->archivePasswordTextField->setText(QString::fromStdString(randomPassword));
}

void SettingsWidget::OnAutoArchivePasswordToggled() {
    qDebug() << "Auto gen archive password toggled";

    auto state = ui->autoGenerateArchivePasswordCheckBox->isChecked();

    ui->archivePasswordTextField->setDisabled(state);
    ui->generateRandomArchivePasswordButton->setDisabled(state);
}

std::string SettingsWidget::generateRandomString(int length) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<> dist(0, sizeof(alphanum) - 2);

    std::string randomString;
    randomString.reserve(length);
    for (int i = 0; i < length; ++i) {
        randomString += alphanum[dist(engine)];
    }

    return randomString;
}
