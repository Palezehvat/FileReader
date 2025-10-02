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
    bool isPaused;
    std::unordered_map<QString, size_t> progressByFile;

private slots:
    void start();
    void stop();
    void pause();
    void resume();
    void problemsWithInputParams(std::shared_ptr<QList<nGeneralHandler::IncorrectInput>> incorrectParams);
    void changeFolder(QLineEdit* lineEdit, QString message);
    void addLog(const QString& message);
    void setStatusFile(const QFileInfo& file, const size_t& percent);
    void getAllFiles(const QList<QFileInfo>& files);
};
#endif // MAINWINDOW_H
