#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QFileDialog>
#include "generalhandler.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    std::shared_ptr<nGeneralHandler::GeneralHandler> handler;

private slots:
    void start();
    void stop();
    void pause();
    void problemsWithInputParams(std::shared_ptr<QList<nGeneralHandler::IncorrectInput>> incorrectParams);
    void changeFolder(QLineEdit* lineEdit, QString message);
};
#endif // MAINWINDOW_H
