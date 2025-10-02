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
    emit sendLog("The specified parameters have been read");
    return false;
}

void GeneralHandler::start(const QString& key, const bool& isNeedDelete,
                           const nLocalHandler::ConflictMode& conflict, const CommonModeTreatment& mode,
                           const QString& pathOutputFolder, const QString& pathInputFolder,
                           const QString& mask) {
    if (getInputParams(key, isNeedDelete, conflict, mode, pathOutputFolder, pathInputFolder, mask)) {
        return;
    }

    if (mode.mode == ModeTreatment::OneTimeTreatment) {
        findFilesByMask();
    } else {
        timer->start(mode.counterToTimer);
    }
}

void GeneralHandler::pause() {
    for (auto& task : tasks) {
        task->pause();
    }
}

void GeneralHandler::stop() {
    for (auto& task : tasks) {
        task->stop();
    }
}

void GeneralHandler::resume() {
    for (auto& task : tasks) {
        task->resume();
    }
}

void GeneralHandler::startTask(const QFileInfo& file) {
    activeTasks++;
    auto task = std::make_shared<nLocalHandler::LocalHandler>(conflict, key, file,
                                                              dirOutputFolder, isNeedDelete);
    connect(task.get(), &nLocalHandler::LocalHandler::logMessage,
            this, &GeneralHandler::sendLog);
    connect(task.get(), &nLocalHandler::LocalHandler::processStatus,
            this, &GeneralHandler::sendStatusFile);
    tasks.push_back(task);
    connect(task.get(), &nLocalHandler::LocalHandler::finished, this, [this]() {
        activeTasks--;
        if (activeTasks == 0) {
            cycleInProgress = false;
        }
    });
    pool->start(task.get());
}

void GeneralHandler::findFilesByMask() {
    if (cycleInProgress) {
        emit sendLog("The counter iteration was skipped because there are already a number of tasks");
        return;
    }
    cycleInProgress = true;
    size_t countFiles = 0; // for logs
    QList<QFileInfo> files;
    for (const QFileInfo& file : dirOutputFolder.entryInfoList(QDir::Files)) {
        if (masks.contains(file.suffix())) {
            files.append(file);
             ++countFiles;
        }
    }

    emit findFiles(files);
    emit sendLog(QString::fromStdString("Multiple files matching the mask were found: "
                                        + std::to_string(countFiles)));

    for (const QFileInfo &file : files) {
        startTask(file);
    }
}

}
