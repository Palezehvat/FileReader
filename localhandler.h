/**
 * @file localhandler.h
 * @brief Responsible for processing a single file
 */
#ifndef LOCALHANDLER_H
#define LOCALHANDLER_H

#include <QRunnable>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <atomic>
#include <QThread>

/**
 * @namespace nLocalHandler
 * @brief Contains class LocalHandler and enum ConflictMode
 */
namespace nLocalHandler {

/**
 * @enum ConflictMode
 * @brief Specifies how to handle the specified files during a name conflict. Two options: overwriting or adding a counter
 */
enum class ConflictMode {
    Overwrite,
    AddCounter
};

class LocalHandler : public QObject, public QRunnable {
    Q_OBJECT

    ConflictMode conflict;
    QString key;
    QFileInfo file;
    QDir folderForOutputFiles;
    bool isNeedDelete;
    std::atomic<bool>& paused;
    std::atomic<bool>& stopped;
    size_t percent;
    static const qint64 blockSize = 1024 * 1024; // 1 MB in bytes
public:
    /**
     * @brief LocalHandler Constructor
     * @param conflict Parameter that specifies what names the source files should have
     * @param key Key is 8 bytes in HEX format. It must begin with the following format: 0x
     * @param file File obtained using a mask specified by the user
     * @param folderForOutputFiles Directory where you need to put the modified file
     * @param isNeedDelete Flag that indicating whether the original files should be deleted
     * @param paused A variable indicating that the user has pressed pause
     * @param stopped A variable indicating that the user pressed stop
     */
    LocalHandler(const ConflictMode& conflict, const QString& key,
                 const QFileInfo& file, const QDir& folderForOutputFiles,
                 const bool& isNeedDelete, std::atomic<bool>& paused, std::atomic<bool>& stopped);
    /**
     * @brief run The key function of the class. Within it, a block-by-block XOR operation is performed on the transferred file data.
     * The parent thread is also notified of success (this information is later passed to the UI).
     */
    void run() override;

signals:
    /**
     * @brief processStatus Transmits information about the percentage of completion of work on a file
     * @param file Information about the input file to be processed
     * @param percent Rate of work completed on a file
     */
    void processStatus(const QFileInfo& file, const size_t& percent);
    /**
     * @brief logMessage Passes information up
     * @param message Why the message was sent
     */
    void logMessage(const QString& message);
    /**
     * @brief finished Notifies the parent thread that the work has completed. This is necessary so that subsequent tasks can begin.
     * @param task Pointer to the class object itself
     */
    void finished(LocalHandler* task);

};

}

#endif // LOCALHANDLER_H
