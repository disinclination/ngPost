#include "PathSettingsWidget.h"
#include "ui_PathSettingsWidget.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>

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

    // Disable save button
    ui->saveButton->setEnabled(false);

    // Catch changes on the UI
    connect(ui->tempPathTextField, &QLineEdit::textChanged, this, &PathSettingsWidget::OnValueChanged);

    // Button related
    connect(ui->cancelButton, &QPushButton::clicked, this, &PathSettingsWidget::HandleCancel);
    connect(ui->tempPathBrowseButton, &QPushButton::clicked, this, &PathSettingsWidget::OnTempPathButtonClicked);
    connect(ui->nzbPathBrowseButton, &QPushButton::clicked, this, &PathSettingsWidget::OnNzbPathButtonClicked);
    connect(ui->rarPathBrowseButton, &QPushButton::clicked, this, &PathSettingsWidget::OnRarPathButtonClicked);
    connect(ui->par2PathBrowseButton, &QPushButton::clicked, this, &PathSettingsWidget::OnPar2PathButtonClicked);
    connect(ui->postHistoryPathBrowseButton, &QPushButton::clicked, this, &PathSettingsWidget::OnPostHistoryPathButtonClicked);
    connect(ui->logPathBrowseButton, &QPushButton::clicked, this, &PathSettingsWidget::OnLogPathButtonClicked);

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

bool PathSettingsWidget::hasChanges() const
{
    return _valueHasChanged;
}

void PathSettingsWidget::OnValueChanged()
{
    _valueHasChanged = true;

    ui->saveButton->setEnabled(true);
}

void PathSettingsWidget::selectDirectory(QLineEdit *lineEdit)
{
    // Open the folder selection dialog
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Folder"), QString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    // Check if the user has selected a directory
    if (!dir.isEmpty()) {
        lineEdit->setText(dir);  // Set the selected directory path to the provided QLineEdit
    }
}

void PathSettingsWidget::OnTempPathButtonClicked()
{
    selectDirectory(ui->tempPathTextField);
}

void PathSettingsWidget::OnNzbPathButtonClicked()
{
    selectDirectory(ui->nzbPathTextField);
}

void PathSettingsWidget::OnRarPathButtonClicked()
{
    selectDirectory(ui->rarPathTextField);
}

void PathSettingsWidget::OnPar2PathButtonClicked()
{
    selectDirectory(ui->par2PathTextField);
}

void PathSettingsWidget::OnPostHistoryPathButtonClicked()
{
    selectDirectory(ui->postHistoryPathTextField);
}

void PathSettingsWidget::OnLogPathButtonClicked()
{
    selectDirectory(ui->logPathTextField);
}