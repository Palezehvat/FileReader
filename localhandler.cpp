#include "localhandler.h"
#include <iostream>

namespace nLocalHandler {

LocalHandler::LocalHandler(const ConflictMode& conflict, const QString& key,
                           const QFileInfo& file, const QDir& folderForOutputFiles,
                           const bool& isNeedDelete) :
    conflict(conflict), key(key), file(file), folderForOutputFiles(folderForOutputFiles),
    isNeedDelete(isNeedDelete), paused(false), stopped(false), percent(0) {}

void LocalHandler::run() {
    QFile input(file.absoluteFilePath());
    if (!input.open(QIODevice::ReadOnly)) {
        emit logMessage(QString::fromStdString(
            "Failed to open file when it was created/overwritten: " + input.fileName().toStdString()
        ));
        return;
    }

    QString outputNameFile = file.fileName();
    if (conflict == ConflictMode::AddCounter) {
        int counter = 1;
        while (QFile::exists(folderForOutputFiles.filePath(outputNameFile))) {
            outputNameFile = file.completeBaseName() + "_" + QString::number(counter) + "." + file.suffix();
            ++counter;
        }
    }

    QFile output(folderForOutputFiles.filePath(outputNameFile));
    if (!output.open(QIODevice::WriteOnly)) {
        emit logMessage(QString::fromStdString(
            "Failed to open file when it was created/overwritten: " + output.fileName().toStdString()
        ));
        return;
    }

    qint64 sizeFile = file.size();
    qint64 processed = 0;

    QByteArray keyBytes = key.toUtf8();

    while (!input.atEnd()) {
        if (stopped.load()) {
            emit finished();
            return;
        }

        while (paused.load()) {
            QThread::msleep(100);
        }

        auto block = input.read(blockSize);
        for (size_t i = 0; i < block.size(); ++i) {
            block[i] = block[i] ^ keyBytes[i % 8];
        }

        output.write(block);
        processed += blockSize;
        emit processStatus(file, static_cast<size_t>((double)processed / sizeFile * 100));
    }

    if (isNeedDelete && !input.remove()) {
        emit logMessage(QString::fromStdString(
            "Input file was deleteed: " + input.fileName().toStdString()
        ));
    }
    emit finished();
}

void LocalHandler::stop() {
    stopped = true;
}

void LocalHandler::pause() {
    paused = true;
}

void LocalHandler::resume() {
    paused = false;
}

}
