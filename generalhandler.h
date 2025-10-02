#ifndef GENERALHANDLER_H
#define GENERALHANDLER_H

#include <QString>
#include <QDir>
#include <QStringList>
#include <QRegularExpression>
#include <QList>
#include <QThreadPool>
#include <QTimer>
#include "localhandler.h"

namespace nGeneralHandler {

enum class IncorrectInput {
    Key,
    Mask,
    InputFolder,
    OutputFolder
};

enum class ModeTreatment {
    OneTimeTreatment,
    TimerTreatment
};

struct CommonModeTreatment {
    size_t counterToTimer;
    ModeTreatment mode;
};

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
    QVector<std::shared_ptr<nLocalHandler::LocalHandler>> tasks;
    QTimer* timer;
    int timerValue;
    std::atomic<int> activeTasks = 0;
    bool cycleInProgress;

public:
    GeneralHandler(QObject *parent = nullptr);
    void start(const QString& key, const bool& isNeedDelete,
               const nLocalHandler::ConflictMode& conflict, const CommonModeTreatment& mode,
               const QString& pathOutputFolder, const QString& pathInputFolder,
               const QString& mask);
    void pause();
    void resume();
    void stop();

private:
    bool getInputParams(const QString& key, const bool& isNeedDelete,
                        const nLocalHandler::ConflictMode& conflict, const CommonModeTreatment& mode,
                        const QString& pathOutputFolder, const QString& pathInputFolder,
                        const QString& mask);
    void findFilesByMask();
    void startTask(const QFileInfo& file);

signals:
    void incorrect(std::shared_ptr<QList<IncorrectInput>> incorrectParams);
    void sendLog(const QString& message);
    void sendStatusFile(const QFileInfo& file, const size_t& percent);
    void findFiles(const QList<QFileInfo>& files);

};

}

#endif // GENERALHANDLER_H
