#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QThread>
#include "protocol.h"
#include "sharedfilefriendlist.h"
#include "uploadfileprogress.h"

class FileSystem : public QWidget
{
    Q_OBJECT
public:
    explicit FileSystem(QWidget *parent = nullptr);
    void updateFileList(PDU* pdu);

    QString strTryEntryDir() const;
    void setStrTryEntryDir(const QString &strTryEntryDir);
    void clearStrTryEntryDir();

    QString getStrSharedFilePath() const;
    QString getStrSharedFileName() const;

    static void operateFileOrDirUpFileListWidget();

    QTimer* getTimer();
    void clearUploadFilePath();
    QString getUploadFilePath();

    QString getDownloadFileName();


public slots:
    void createDir();
    void flushDir();
    void delFileOrDir();
    void renameFile();
    void entryDir(const QModelIndex &index);
    void returnPreDir();
    void uploadFile();
    void startTimer();
    void uploadFileData();
    void downloadFile();
    void moveFile();
    void moveDesDir();
    void shareFile();
    TransFile* getDownloadFileInfo();

signals:

private:
    QListWidget *m_pFileListW = nullptr;

    QPushButton *m_pReturnPrePB = nullptr;
    QPushButton *m_pCreateDirPB = nullptr;
    QPushButton *m_pFlushDirPB = nullptr;
    QPushButton *m_pDelFileOrDirPB = nullptr;

    QPushButton *m_pRenameFilePB = nullptr;
    QPushButton *m_pUploadFilePB = nullptr;
    QPushButton *m_pDownloadFilePB = nullptr;
    QPushButton *m_pShareFilePB = nullptr;

    QPushButton *m_pMoveFilePB = nullptr;
    QPushButton *m_pMoveDesDirDB = nullptr;

    QString m_strTryEntryDir;

    QString m_strUploadFilePath;
    QTimer *m_pTimer = nullptr;

    TransFile *m_downloadFile = nullptr;
    QString m_downloadFileName;

    QString m_strMoveFileName;
    QString m_strMoveOldDir;

    QString m_strSharedFilePath;
    QString m_strSharedFileName;
    sharedFileFriendList *m_pSharedFileFLW;
};

#endif // FILESYSTEM_H
