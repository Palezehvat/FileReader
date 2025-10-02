#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    handler = std::make_shared<nGeneralHandler::GeneralHandler>(this);
    isPaused = false;

    QRegularExpression hexRegex("0x[0-9A-Fa-f]{16}");
    QValidator *validator = new QRegularExpressionValidator(hexRegex, this);
    ui->lineEditOfKey->setValidator(validator);

    connect(handler.get(), &nGeneralHandler::GeneralHandler::incorrect, this, &MainWindow::problemsWithInputParams);
    connect(handler.get(), &nGeneralHandler::GeneralHandler::sendLog, this, &MainWindow::addLog);
    connect(handler.get(), &nGeneralHandler::GeneralHandler::sendStatusFile, this, &MainWindow::setStatusFile);
    connect(handler.get(), &nGeneralHandler::GeneralHandler::findFiles, this, &MainWindow::getAllFiles);
    connect(ui->pushButtonOfPause, &QPushButton::clicked, this, [this](){
        if (isPaused) {
            isPaused = false;
            ui->pushButtonOfPause->setText("Pause");
            resume();
        } else {
            isPaused = true;
            ui->pushButtonOfPause->setText("Resume");
            pause();
        }
    });
    connect(ui->lineEditOfInputFolder, &QLineEdit::textChanged, this, [this](){
        ui->labelOfInputFolder->setStyleSheet("QLabel { color : black; }");
    });
    connect(ui->lineEditOfOutputFolder, &QLineEdit::textChanged, this, [this](){
        ui->labelOfOutputFolder->setStyleSheet("QLabel { color : black; }");
    });
    connect(ui->lineEditOfKey, &QLineEdit::textChanged, this, [this](){
        ui->labelOfKey->setStyleSheet("QLabel { color : black; }");
    });
    connect(ui->lineEditOfMaskInputFiles, &QLineEdit::textChanged, this, [this](){
        ui->labelOfMaskInputFiles->setStyleSheet("QLabel { color : black; }");
    });
    connect(ui->pushButtonOfStop, &QPushButton::clicked, this, &MainWindow::stop);
    connect(ui->pushButtonOfStart, &QPushButton::clicked, this, &MainWindow::start);
    connect(ui->pushButtonOfOutputFolder, &QPushButton::clicked, this, [this](){
        changeFolder(ui->lineEditOfOutputFolder, "Select folder for output files");
    });
    connect(ui->pushButtonOfInputFolder, &QPushButton::clicked, this, [this](){
        changeFolder(ui->lineEditOfInputFolder, "Select folder for input files");
    });
}

void MainWindow::start() {
    addLog("The process has been started");
    nLocalHandler::ConflictMode conflict = ui->radioButtonOfCounterForOutputFiles->isChecked()
        ? nLocalHandler::ConflictMode::AddCounter : nLocalHandler::ConflictMode::Overwrite;
    nGeneralHandler::CommonModeTreatment mode;
    if (ui->radioButtonOfOneTimeTreatment->isChecked()) {
        mode = {0, nGeneralHandler::ModeTreatment::OneTimeTreatment};
    } else {
        mode = {static_cast<size_t>(ui->spinBoxOfTimerTreatment->value()), nGeneralHandler::ModeTreatment::OneTimeTreatment};
    }
    handler->start(ui->lineEditOfKey->text(), ui->checkBoxOfDeleteFilesAfterProcess->isChecked(),
                   conflict, mode, ui->lineEditOfOutputFolder->text(),
                   ui->lineEditOfInputFolder->text(), ui->lineEditOfMaskInputFiles->text());
}

void MainWindow::stop() {
    addLog("The process has been stopped");
    handler->stop();
}

void MainWindow::pause() {
    addLog("The process has been paused");
    handler->pause();
}

void MainWindow::resume() {
    addLog("The process has been resume");
    handler->resume();
}

void MainWindow::addLog(const QString& message) {
    ui->textEditOfLogs->append(message);
}

void MainWindow::changeFolder(QLineEdit* lineEdit, QString message) {
    addLog("User Folder Selection. " + message);
    QString dir = QFileDialog::getExistingDirectory(
        this,
        message,
        QDir::homePath()
    );

    if (!dir.isEmpty()) {
        lineEdit->setText(dir);
    }
}

void MainWindow::problemsWithInputParams(std::shared_ptr<QList<nGeneralHandler::IncorrectInput>> incorrectParams) {
    while(!incorrectParams->isEmpty()) {
        switch (incorrectParams->back()) {
        case nGeneralHandler::IncorrectInput::InputFolder:
            incorrectParams->pop_back();
            addLog("Invalid input folder");
            ui->labelOfInputFolder->setStyleSheet("QLabel { color : red; }");
            break;
        case nGeneralHandler::IncorrectInput::OutputFolder:
            incorrectParams->pop_back();
            addLog("Invalid output folder");
            ui->labelOfOutputFolder->setStyleSheet("QLabel { color : red; }");
            break;
        case nGeneralHandler::IncorrectInput::Key:
            incorrectParams->pop_back();
            addLog("Incorrect HEX key");
            ui->labelOfKey->setStyleSheet("QLabel { color : red; }");
            break;
        case nGeneralHandler::IncorrectInput::Mask:
            incorrectParams->pop_back();
            addLog("Incorrect mask");
            ui->labelOfMaskInputFiles->setStyleSheet("QLabel { color : red; }");
            break;
        }
    }
}

void MainWindow::setStatusFile(const QFileInfo& file, const size_t& percent) {
    progressByFile[file.fileName()] = percent;
    size_t sum = 0;
    for (auto& p : progressByFile) sum += p.second;
    size_t total = progressByFile.empty() ? 0 : sum / progressByFile.size();

    ui->progressBarOfTreatment->setValue(total);

    for (int i = 0; i < ui->listWidgetOfStatusTreatment->count(); ++i) {
        QListWidgetItem *item = ui->listWidgetOfStatusTreatment->item(i);
        if (item->data(Qt::UserRole).toString() == file.fileName()) {
            item->setText(QString("%1 — %2%").arg(file.fileName()).arg(percent));
            return;
        }
    }
}

void MainWindow::getAllFiles(const QList<QFileInfo>& files) {
    ui->listWidgetOfStatusTreatment->clear();
    progressByFile.clear();
    ui->progressBarOfTreatment->setValue(0);
    for (const QFileInfo &file : files) {
        progressByFile[file.fileName()] = 0;
        QListWidgetItem *item = new QListWidgetItem(
            QString("%1 — 0%").arg(file.fileName()),
            ui->listWidgetOfStatusTreatment
        );
        item->setData(Qt::UserRole, file.fileName());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
