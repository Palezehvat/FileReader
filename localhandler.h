#ifndef LOCALHANDLER_H
#define LOCALHANDLER_H

#include <QRunnable>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <atomic>
#include <QThread>

namespace nLocalHandler {

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
    std::atomic<bool> paused;
    std::atomic<bool> stopped;
    size_t percent;
    const qint64 blockSize = 4 * 1024 * 1024; // 4 MB in bytes
public:
    LocalHandler(const ConflictMode& conflict, const QString& key,
                 const QFileInfo& file, const QDir& folderForOutputFiles,
                 const bool& isNeedDelete);
    void run() override;
    void pause();
    void resume();
    void stop();

signals:
    void processStatus(const QFileInfo& file, size_t percent);
    void logMessage(QString message);

};

}

#endif // LOCALHANDLER_H
