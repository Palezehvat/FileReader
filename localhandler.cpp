#include "localhandler.h"
#include <iostream>

namespace nLocalHandler {

LocalHandler::LocalHandler(const ConflictMode& conflict, const QString& key,
                           const QFileInfo& file, const QDir& folderForOutputFiles,
                           const bool& isNeedDelete, std::atomic<bool>& paused, std::atomic<bool>& stopped) :
    QObject(nullptr), QRunnable(), conflict(conflict), key(key), file(file),
    folderForOutputFiles(folderForOutputFiles), isNeedDelete(isNeedDelete),
    percent(0), paused(paused), stopped(stopped) {}

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
    } else {
        outputNameFile = file.fileName() + ".tmp";
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
    QElapsedTimer timer;
    timer.start();

    while (!input.atEnd()) {
        if (stopped.load()) {
            emit finished(this);
            input.close();
            output.close();
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
        processed += block.size();
        size_t newPercent = static_cast<size_t>((double)processed / sizeFile * 100);
        if (timer.elapsed() > 100 && newPercent != percent) {
            percent = newPercent;
            timer.restart();
            emit processStatus(file, percent);
        }
    }
    emit processStatus(file, 100);
    input.close();
    output.close();

    if (isNeedDelete || ConflictMode::Overwrite == conflict && !isNeedDelete) {
        if (!input.remove()) {
            emit logMessage(QString::fromStdString(
                "Input file was deleteed: " + input.fileName().toStdString()
            ));
        }
    }

    if (conflict == ConflictMode::Overwrite && !output.rename(file.absoluteFilePath())) {
        emit logMessage("Failed to rename " + output.fileName() + " в " + file.fileName());
    }

    emit finished(this);
}

}
