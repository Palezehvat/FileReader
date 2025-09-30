#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    handler = std::make_shared<nGeneralHandler::GeneralHandler>(this);

    QRegularExpression hexRegex("0x[0-9A-Fa-f]{16}");
    QValidator *validator = new QRegularExpressionValidator(hexRegex, this);
    ui->lineEditOfKey->setValidator(validator);

    connect(handler.get(), &nGeneralHandler::GeneralHandler::incorrect, this, &MainWindow::problemsWithInputParams);
    connect(ui->pushButtonOfStart, &QPushButton::clicked, this, &MainWindow::start);
    connect(ui->pushButtonOfOutputFolder, &QPushButton::clicked, this, [this](){
        changeFolder(ui->lineEditOfOutputFolder, "Select folder for output files");
    });
    connect(ui->pushButtonOfInputFolder, &QPushButton::clicked, this, [this](){
        changeFolder(ui->lineEditOfInputFolder, "Select folder for input files");
    });
}

void MainWindow::start() {
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
    handler->stop();
}

void MainWindow::pause() {
    handler->pause();
}

void MainWindow::changeFolder(QLineEdit* lineEdit, QString message) {
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
            break;
        case nGeneralHandler::IncorrectInput::OutputFolder:
            incorrectParams->pop_back();
            break;
        case nGeneralHandler::IncorrectInput::Key:
            incorrectParams->pop_back();
            break;
        case nGeneralHandler::IncorrectInput::Mask:
            incorrectParams->pop_back();
            break;
        }
    }
    // Подсветить кнопки, что тут проблемы(не забыть выключение подсветки)
}

MainWindow::~MainWindow()
{
    delete ui;
}
