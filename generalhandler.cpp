#include "generalhandler.h"
#include <iostream>

namespace nGeneralHandler {

GeneralHandler::GeneralHandler(QObject *parent) : QObject(parent) {
    incorrectParams = std::make_shared<QList<IncorrectInput>>();
    pool = QThreadPool::globalInstance();
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        findFilesByMask();
    });
}

bool GeneralHandler::getInputParams(const QString& key, const bool& isNeedDelete,
                                    const nLocalHandler::ConflictMode& conflict, const CommonModeTreatment& mode,
                                    const QString& pathOutputFolder, const QString& pathInputFolder,
                                    const QString& mask) {
    dirOutputFolder = QDir(pathOutputFolder);
    dirInputFolder = QDir(pathInputFolder);

    if (!dirOutputFolder.exists() || pathOutputFolder == "") {
        incorrectParams->append(IncorrectInput::OutputFolder);
    }

    if (!dirInputFolder.exists() || pathInputFolder == "") {
        incorrectParams->append(IncorrectInput::InputFolder);
    }

    if (key.length() != 18) {
        incorrectParams->append(IncorrectInput::Key);
    }

    masks = mask.split(QRegularExpression("[,; ]+"), Qt::SkipEmptyParts);
    for (auto& m : masks) {
        if (m.startsWith("*.")) {
            m.remove(0, 2);
        }
    }

    if (masks.isEmpty()) {
        incorrectParams->append(IncorrectInput::Mask);
    }

    if (!incorrectParams->isEmpty()) {
        emit incorrect(incorrectParams);
        emit sendLog("The specified parameters have been read and are not correct");
        return true;
    }

    this->key = key;
    this->isNeedDelete = isNeedDelete;
    this->conflict = conflict;
    this->mode = mode;
    this->timerValue = timerValue;
    cycleInProgress = false;
    paused.store(false);
    stopped.store(false);
    activeTasks.store(0);
    emit sendLog("The specified parameters have been read");
    return false;
}

void GeneralHandler::start(const QString& key, const bool& isNeedDelete,
                           const nLocalHandler::ConflictMode& conflict, const CommonModeTreatment& mode,
                           const QString& pathOutputFolder, const QString& pathInputFolder,
                           const QString& mask) {
    stopped.store(true);
    stopped.store(false);
    if (getInputParams(key, isNeedDelete, conflict, mode, pathOutputFolder, pathInputFolder, mask)) {
        return;
    }
    if (mode.mode == ModeTreatment::OneTimeTreatment) {
        findFilesByMask();
    } else {
        timer->start(mode.counterToTimer * 1000);
    }
}

void GeneralHandler::pause() {
    paused.store(true);
}

void GeneralHandler::stop() {
    stopped.store(true);
}

void GeneralHandler::resume() {
    paused.store(false);
}

void GeneralHandler::startTasks(const QList<QFileInfo>& files) {
    const int maxTaskInMoment = std::max(1, pool->maxThreadCount() * 4);
    auto activeCount = std::make_shared<std::atomic<int>>(0);

    std::thread([this, files, maxTaskInMoment, activeCount]() {
        size_t idx = 0;
        const size_t total = files.size();

        while (idx < total) {
            if (stopped.load()) {
                if (mode.mode == ModeTreatment::TimerTreatment) {
                    timer->stop();
                }
                break;
            }
            while (paused.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (stopped.load()) break;
            }

            if (activeCount->load() >= maxTaskInMoment) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            const QFileInfo file = files.at(idx++);
            auto* task = new nLocalHandler::LocalHandler(conflict, key, file, dirOutputFolder, isNeedDelete, paused, stopped);
            task->setAutoDelete(true);

            connect(task, &nLocalHandler::LocalHandler::logMessage, this, &GeneralHandler::sendLog, Qt::QueuedConnection);
            connect(task, &nLocalHandler::LocalHandler::processStatus, this, &GeneralHandler::sendStatusFile, Qt::QueuedConnection);

            activeCount->fetch_add(1);
            connect(task, &nLocalHandler::LocalHandler::finished, this, [activeCount, this](nLocalHandler::LocalHandler*) {
                activeCount->fetch_sub(1);
            }, Qt::QueuedConnection);

            pool->start(task);
        }

        while (activeCount->load() > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        QMetaObject::invokeMethod(this, [this]() {
            cycleInProgress = false;
        }, Qt::QueuedConnection);
    }).detach();
}

void GeneralHandler::findFilesByMask() {
    if (cycleInProgress) return;
    cycleInProgress = true;

    QList<QFileInfo> files;
    for (const QFileInfo& file : dirOutputFolder.entryInfoList(QDir::Files)) {
        if (masks.contains(file.suffix()) || masks.contains(file.fileName())) {
            files.append(file);
        }
    }

    emit findFiles(files);
    emit sendLog(QString("Found %1 files").arg(files.size()));

    startTasks(files);
}

}
