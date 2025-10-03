#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QSignalSpy>
#include <QCoreApplication>
#include "generalhandler.h"
#include "localhandler.h"

class TestableHandler : public nGeneralHandler::GeneralHandler {
public:
    using nGeneralHandler::GeneralHandler::getInputParams;
    using nGeneralHandler::GeneralHandler::findFilesByMask;
protected:
    void startTasks(const QList<QFileInfo>& files) override {}
};

TEST(GeneralHandlerTest, InvalidFolders) {
    TestableHandler handler;

    nGeneralHandler::CommonModeTreatment mode{0, nGeneralHandler::ModeTreatment::OneTimeTreatment};
    bool result = handler.getInputParams(
        "0x1234567890ABCDEF",
        false,
        nLocalHandler::ConflictMode::Overwrite,
        mode,
        "",
        "",
        "*.txt"
    );

    EXPECT_TRUE(result);
}

TEST(GeneralHandlerTest, ValidParameters) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    QFile file1(tempDir.path() + "/file1.txt");
    QFile file2(tempDir.path() + "/file2.log");

    TestableHandler handler;
    nGeneralHandler::CommonModeTreatment mode{0, nGeneralHandler::ModeTreatment::OneTimeTreatment};

    bool result = handler.getInputParams(
        "0x1234567890ABCDEF",
        false,
        nLocalHandler::ConflictMode::Overwrite,
        mode,
        tempDir.path(),
        tempDir.path(),
        "*.txt"
    );

    EXPECT_FALSE(result);
}

TEST(GeneralHandlerTest, InvalidKey) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    QFile file1(tempDir.path() + "/file1.txt");
    QFile file2(tempDir.path() + "/file2.log");

    TestableHandler handler;
    nGeneralHandler::CommonModeTreatment mode{0, nGeneralHandler::ModeTreatment::OneTimeTreatment};

    bool result = handler.getInputParams(
        "0x1234567890",
        false,
        nLocalHandler::ConflictMode::Overwrite,
        mode,
        tempDir.path(),
        tempDir.path(),
        "*.txt"
    );

    EXPECT_TRUE(result);
}

TEST(GeneralHandlerTest, FindAllFilesByMask) {
    int argc = 0;
    char** argv = nullptr;
    QCoreApplication app(argc, argv);
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    QFile file1(tempDir.path() + "/file1.txt");
    QFile file2(tempDir.path() + "/file2.txt");
    QFile file3(tempDir.path() + "/file3.log");
    QFile file4(tempDir.path() + "/file4.log");
    file1.open(QIODevice::WriteOnly);
    file1.close();
    file2.open(QIODevice::WriteOnly);
    file2.close();
    file3.open(QIODevice::WriteOnly);
    file3.close();
    file4.open(QIODevice::WriteOnly);
    file4.close();

    TestableHandler handler;
    nGeneralHandler::CommonModeTreatment mode{0, nGeneralHandler::ModeTreatment::OneTimeTreatment};

    QSignalSpy spy(&handler, &nGeneralHandler::GeneralHandler::findFiles);

    bool result = handler.getInputParams(
        "0x1234567890ABCDEF",
        false,
        nLocalHandler::ConflictMode::Overwrite,
        mode,
        tempDir.path(),
        tempDir.path(),
        "*.txt; file3.log"
    );

    handler.findFilesByMask();

    EXPECT_EQ(spy.count(), 1);

    QList<QVariant> arguments = spy.takeFirst();
    QList<QFileInfo> files = qvariant_cast<QList<QFileInfo>>(arguments.at(0));

    EXPECT_EQ(files.size(), 3);
    EXPECT_EQ(files[0].fileName(), "file1.txt");
    EXPECT_EQ(files[1].fileName(), "file2.txt");
    EXPECT_EQ(files[2].fileName(), "file3.log");
}

TEST(LocalHandlerTest, CorrectFileConversion) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    QString filePath = tempDir.path() + "/file.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("Hello world!");
    file.close();

    std::atomic<bool> stopped{false};
    std::atomic<bool> paused{false};
    nLocalHandler::LocalHandler handler = nLocalHandler::LocalHandler(nLocalHandler::ConflictMode::Overwrite, "0x1234567890ABCDEF", QFileInfo(file.fileName()),
                                                                      QDir(tempDir.path()), false, paused, stopped);

    handler.run();
    QFile checkFileAfterFirstRun(filePath);
    ASSERT_TRUE(checkFileAfterFirstRun.open(QIODevice::ReadOnly));
    QByteArray dataAfterFirstRun = checkFileAfterFirstRun.readAll();
    checkFileAfterFirstRun.close();
    EXPECT_NE(dataAfterFirstRun, QByteArray("Hello world!"));
    handler.run();

    QFile checkFileAfterSecondRun(filePath);
    ASSERT_TRUE(checkFileAfterSecondRun.open(QIODevice::ReadOnly));
    QByteArray dataAfterSecondRun = checkFileAfterSecondRun.readAll();
    checkFileAfterSecondRun.close();
    EXPECT_EQ(dataAfterSecondRun, QByteArray("Hello world!"));
}

TEST(LocalHandlerTest, DeleteFileAfterConversionIsCorrect) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    QString filePath = tempDir.path() + "/file.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("Hello world!");
    file.close();

    std::atomic<bool> stopped{false};
    std::atomic<bool> paused{false};
    nLocalHandler::LocalHandler handler = nLocalHandler::LocalHandler(nLocalHandler::ConflictMode::AddCounter, "0x1234567890ABCDEF", QFileInfo(file.fileName()),
                                                                      QDir(tempDir.path()), true, paused, stopped);

    handler.run();
    QFileInfo fileInfo(file.fileName());
    ASSERT_FALSE(fileInfo.exists());
    QFileInfo resultFile(tempDir.path() + "/file_1.txt");
    EXPECT_TRUE(resultFile.exists());
}
