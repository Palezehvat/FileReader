/**
 * @file mainwindow.h
 * @brief Contains work on how the main window works
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QFileDialog>
#include "generalhandler.h"

QT_BEGIN_NAMESPACE
/**
 * @namespace UI
 * @brief Automatically generated namespace based on ui file
 */
namespace Ui {
/**
 * @class MainWindow
 * @brief Generated main window interface class
 */
class MainWindow;
}
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief The class responsible for the operation of the main window
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief MainWindow Constructor
     * @param parent The parent QObject. If specified, the object will be automatically destroyed along with the parent
     */
    MainWindow(QWidget *parent = nullptr);
    /**
     * @brief Destructor
     */
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    std::shared_ptr<nGeneralHandler::GeneralHandler> handler;
    bool isPaused;
    std::unordered_map<QString, size_t> progressByFile;

private slots:
    /**
     * @brief start Reacts to the user pressing the start button
     */
    void start();
    /**
     * @brief stop Reacts to the user pressing the stop button
     */
    void stop();
    /**
     * @brief pause Reacts to the user pressing the pause button the first time
     */
    void pause();
    /**
     * @brief resume Reacts to the user pressing the start button the second time
     */
    void resume();
    /**
     * @brief problemsWithInputParams Highlights fields that were filled in incorrectly by the user
     * @param incorrectParams Invalid parameters specified by the user
     */
    void problemsWithInputParams(std::shared_ptr<QList<nGeneralHandler::IncorrectInput>> incorrectParams);
    /**
     * @brief changeFolder Opens a folder selection window
     * @param lineEdit The place where the full path to the selected folder will be written
     * @param message The message that should be sent to the user when this window is opened
     */
    void changeFolder(QLineEdit* lineEdit, QString message);
    /**
     * @brief addLog Shows the user logs that occur as a result of the application's operation
     * @param message The message that should be shown to the user
     */
    void addLog(const QString& message);
    /**
     * @brief setStatusFile Processing the signal about the percentage of work completed on a file
     * @param file The file being processed
     * @param percent Percentage of completed processing
     */
    void setStatusFile(const QFileInfo& file, const size_t& percent);
    /**
     * @brief getAllFiles Getting all found files matching the mask
     * @param files Files found
     */
    void getAllFiles(const QList<QFileInfo>& files);
};
#endif // MAINWINDOW_H
