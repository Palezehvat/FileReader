#include "generalhandler.h"

namespace nGeneralHandler {

GeneralHandler::GeneralHandler(QObject *parent) : QObject(parent) {
    incorrectParams = std::make_shared<QList<IncorrectInput>>();
    pool = QThreadPool::globalInstance();
}

bool GeneralHandler::getInputParams(const QString& key, const bool& isNeedDelete,
                                    const nLocalHandler::ConflictMode& conflict, const CommonModeTreatment& mode,
                                    const QString& pathOutputFolder, const QString& pathInputFolder,
                                    const QString& mask) {
    dirOutputFolder = QDir(pathOutputFolder);
    dirInputFolder = QDir(pathInputFolder);


    if (!dirOutputFolder.exists()) {
        incorrectParams->append(IncorrectInput::OutputFolder);
    }

    if (!dirInputFolder.exists()) {
        incorrectParams->append(IncorrectInput::InputFolder);
    }

    if (key.length() != 18) {
        incorrectParams->append(IncorrectInput::Key);
    }

    masks = mask.split(QRegularExpression("[,; ]+"), Qt::SkipEmptyParts);
    if (masks.isEmpty()) {
        incorrectParams->append(IncorrectInput::Mask);
    }

    if (incorrectParams->isEmpty()) {
        emit incorrect(incorrectParams);
        return true;
    }

    this->key = key;
    this->isNeedDelete = isNeedDelete;
    this->conflict = conflict;
    this->mode = mode;
    return false;
}

void GeneralHandler::start(const QString& key, const bool& isNeedDelete,
                           const nLocalHandler::ConflictMode& conflict, const CommonModeTreatment& mode,
                           const QString& pathOutputFolder, const QString& pathInputFolder,
                           const QString& mask) {
    if (getInputParams(key, isNeedDelete, conflict, mode, pathOutputFolder, pathInputFolder, mask)) {
        return;
    }

   findFilesByMask();
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
    auto task = std::make_shared<nLocalHandler::LocalHandler>(conflict, key, file,
                                                              dirOutputFolder, isNeedDelete);
    tasks.push_back(task);
    pool->start(task.get());
}

void GeneralHandler::findFilesByMask() {
    size_t countFiles = 0; // для логов
    for (const QFileInfo& file : dirOutputFolder.entryInfoList(QDir::Files)) {
        if (masks.contains(file.suffix())) {
            ++countFiles; // для логов
            startTask(file);
        }
    }
}

}
