/**
 * @file generalhandler.h
 * @brief application logic related to UI
 */
#ifndef GENERALHANDLER_H
#define GENERALHANDLER_H

#include <QString>
#include <QDir>
#include <QStringList>
#include <QRegularExpression>
#include <QList>
#include <QThreadPool>
#include <QTimer>
#include <thread>
#include <chrono>
#include "localhandler.h"

/**
 * @namespace nGeneralHandler
 * @brief Contains class GeneralHandler, enum IncorrectInput, enum ModeTreatment and struct CommonModeTreatment
 */
namespace nGeneralHandler {

/**
 * @enum IncorrectInput
 * @brief It is necessary to inform the UI about incorrect input
 */
enum class IncorrectInput {
    Key,
    Mask,
    InputFolder,
    OutputFolder
};

/**
 * @enum ModeTreatment
 * @brief Indicates which operating mode the user has selected: single start or timer
 */
enum class ModeTreatment {
    OneTimeTreatment,
    TimerTreatment
};

/**
 * @struct CommonModeTreatment
 * @brief Contains information about the selected operating mode and also the frequency of the timer start
 */
struct CommonModeTreatment {
    size_t counterToTimer;
    ModeTreatment mode;
};

/**
 * @class GeneralHandler
 * @brief The class responsible for processing input parameters. Contains a child thread that works with QThreadPool
 */
class GeneralHandler : public QObject {
    Q_OBJECT

    QString key;
    bool isNeedDelete;
    nLocalHandler::ConflictMode conflict;
    CommonModeTreatment mode;
    QDir dirOutputFolder;
    QDir dirInputFolder;
    QStringList masks;
    std::shared_ptr<QList<IncorrectInput>> incorrectParams;
    QThreadPool* pool;
    QTimer* timer;
    int timerValue;
    std::atomic<int> activeTasks;
    std::atomic<bool> paused;
    std::atomic<bool> stopped;
    bool cycleInProgress;

public:
    /**
     * @brief GeneralHandler
     * @param parent The parent QObject. If specified, the object will be automatically destroyed along with the parent
     */
    GeneralHandler(QObject *parent = nullptr);
    /**
     * @brief start Main function of the class, responsible for starting the logic
     * @param key Key is 8 bytes in HEX format. It must begin with the following format: 0x
     * @param isNeedDelete Flag that indicating whether the original files should be deleted
     * @param conflict Parameter that specifies what names the source files should have
     * @param mode Determines the program's operating mode: single run or timer
     * @param pathOutputFolder Specifies from which folder the files will be taken
     * @param pathInputFolder Specifies which folder to write files to
     * @param mask Indicates which files to take, two recording options: *.txt;(also *.txt,) or if you want specific file: fileName.txt
     */
    void start(const QString& key, const bool& isNeedDelete,
               const nLocalHandler::ConflictMode& conflict, const CommonModeTreatment& mode,
               const QString& pathOutputFolder, const QString& pathInputFolder,
               const QString& mask);
    /**
     * @brief Pauses the process
     */
    void pause();
    /**
     * @brief Resumes the process
     */
    void resume();
    /**
     * @brief Completely stops working. IMPORTANT: Files that have not been completely modified will be incomplete
     */
    void stop();

protected:
    /**
     * @brief getInputParams Function aimed at reading and checking parameters transmitted from the UI
     * @param key Key is 8 bytes in HEX format. It must begin with the following format: 0x
     * @param isNeedDelete Flag that indicating whether the original files should be deleted
     * @param conflict Parameter that specifies what names the source files should have
     * @param mode Determines the program's operating mode: single run or timer
     * @param pathOutputFolder Specifies from which folder the files will be taken
     * @param pathInputFolder Specifies which folder to write files to
     * @param mask Indicates which files to take, two recording options: *.txt;(also *.txt,) or if you want specific file: fileName.txt
     * @return returns True if there are invalid parameters else false
     */
    bool getInputParams(const QString& key, const bool& isNeedDelete,
                        const nLocalHandler::ConflictMode& conflict, const CommonModeTreatment& mode,
                        const QString& pathOutputFolder, const QString& pathInputFolder,
                        const QString& mask);
    /**
     * @brief Finds all files matching the mask
     */
    void findFilesByMask();
    /**
     * @brief startTasks Starts a child thread that creates tasks for each of the individual files that match the mask
     * @param files Satisfying the mask passed by the user
     */
    virtual void startTasks(const QList<QFileInfo>& files);

signals:
    /**
     * @brief incorrect Signal to the UI that the data submitted by the user is incorrect
     * @param incorrectParams Incorrect parameters passed by the user
     */
    void incorrect(std::shared_ptr<QList<IncorrectInput>> incorrectParams);
    /**
     * @brief sendLog Sends logs to the UI so the user can monitor the process
     * @param message Log message to user
     */
    void sendLog(const QString& message);
    /**
     * @brief sendStatusFile Forwards information from a child thread
     * @param file The file being worked on
     * @param percent Rate of work completed on a file
     */
    void sendStatusFile(const QFileInfo& file, const size_t& percent);
    /**
     * @brief findFiles Passes information to the UI so it can display progress on files
     * @param files All found files that match the mask specified by the user
     */
    void findFiles(const QList<QFileInfo>& files);

};

}

#endif // GENERALHANDLER_H
