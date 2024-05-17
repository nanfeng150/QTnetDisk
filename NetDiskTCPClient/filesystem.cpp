#include "filesystem.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QIcon>
#include <QList>
#include <QSize>
#include "tcpclient.h"

FileSystem::FileSystem(QWidget *parent) : QWidget(parent)
{
    m_strTryEntryDir.clear();
    m_strUploadFilePath.clear();
    m_pTimer = new QTimer;
    m_downloadFile = new TransFile;
    m_downloadFile->bTransform = false;
    m_pFileListW = new QListWidget;

    m_pReturnPrePB = new QPushButton("返回");
    m_pReturnPrePB->setIcon(QIcon(":/icon/returnBtn.png"));

    m_pCreateDirPB = new QPushButton("创建文件夹");
    m_pCreateDirPB->setIcon(QIcon(":/icon/createDirBtn.png"));

    m_pDelFileOrDirPB = new QPushButton("删除");
    m_pDelFileOrDirPB->setIcon(QIcon(":/icon/deleteBtn.png"));

    m_pFlushDirPB = new QPushButton("刷新文件夹");
    m_pFlushDirPB->setIcon(QIcon(":/icon/updateBtn.png"));

    m_pRenameFilePB = new QPushButton("重命名");
    m_pRenameFilePB->setIcon(QIcon(":/icon/reNameBtn.png"));

    QHBoxLayout *pDirOpVBL = new QHBoxLayout;
    pDirOpVBL -> addWidget(m_pReturnPrePB);
    pDirOpVBL -> addWidget(m_pCreateDirPB);
    pDirOpVBL -> addWidget(m_pFlushDirPB);
    pDirOpVBL -> addWidget(m_pDelFileOrDirPB);
    pDirOpVBL -> addWidget(m_pRenameFilePB);

    m_pUploadFilePB = new QPushButton("上传文件");
    m_pUploadFilePB->setIcon(QIcon(":/icon/uploadFileBtn.png"));

    m_pDownloadFilePB = new QPushButton("下载文件");
    m_pDownloadFilePB->setIcon(QIcon(":/icon/downloadFileBtn.png"));

    m_pShareFilePB = new QPushButton("分享文件");
    m_pShareFilePB->setIcon(QIcon(":/icon/shareFileBtn.png"));

    m_pMoveFilePB = new QPushButton("移动文件");
    m_pMoveFilePB->setIcon(QIcon(":/icon/moveBtn.png"));

    m_pMoveDesDirDB = new QPushButton("目标目录");
    m_pMoveDesDirDB->setIcon(QIcon(":/icon/contentsBtn.png"));

    m_pMoveDesDirDB->setEnabled(false); // 设置目标文件不可点击

    QHBoxLayout *pFileOpVBL = new QHBoxLayout;
    pFileOpVBL -> addWidget(m_pUploadFilePB);
    pFileOpVBL -> addWidget(m_pDownloadFilePB);
    pFileOpVBL -> addWidget(m_pShareFilePB);
    pFileOpVBL -> addWidget(m_pMoveFilePB);
    pFileOpVBL -> addWidget(m_pMoveDesDirDB);

    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL -> addLayout(pDirOpVBL);
    pMainVBL -> addLayout(pFileOpVBL);
    pMainVBL -> addWidget(m_pFileListW);

    setLayout(pMainVBL);


    m_pSharedFileFLW = new sharedFileFriendList;
    m_pSharedFileFLW -> setWindowIcon(QIcon(":/icon/fileServer.png"));
    m_pSharedFileFLW -> setWindowTitle("分享文件");
    m_pSharedFileFLW -> setMinimumSize(QSize(620, 590));

    connect(m_pCreateDirPB, SIGNAL(clicked(bool)),
            this, SLOT(createDir()));
    connect(m_pFlushDirPB, SIGNAL(clicked(bool)),
            this, SLOT(flushDir()));
    connect(m_pDelFileOrDirPB, SIGNAL(clicked(bool)),
            this, SLOT(delFileOrDir()));
    connect(m_pRenameFilePB, SIGNAL(clicked(bool)),
            this, SLOT(renameFile()));
    connect(m_pFileListW, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(entryDir(QModelIndex)));
    connect(m_pReturnPrePB, SIGNAL(clicked(bool)),
            this, SLOT(returnPreDir()));
    connect(m_pUploadFilePB, SIGNAL(clicked(bool)),
            this, SLOT(uploadFile()));
    connect(m_pTimer, SIGNAL(timeout()),
            this, SLOT(uploadFileData()));
    connect(m_pDownloadFilePB, SIGNAL(clicked(bool)),
            this, SLOT(downloadFile()));
    connect(m_pMoveFilePB, SIGNAL(clicked(bool)),
            this, SLOT(moveFile()));
    connect(m_pMoveDesDirDB, SIGNAL(clicked(bool)),
            this, SLOT(moveDesDir()));
    connect(m_pShareFilePB, SIGNAL(clicked(bool)),
            this, SLOT(shareFile()));
}

void FileSystem::updateFileList(PDU *pdu)
{
    if(nullptr == pdu)
    {
        return ;
    }
    uint uiFileNum = pdu -> uiMsgLen / sizeof(FileInfo);
    FileInfo *pFileInfo = nullptr;

    m_pFileListW -> clear();
    QList<QListWidgetItem *> dirItem;
    QList<QListWidgetItem *> fileItem;

    for(uint i = 0; i < uiFileNum; ++ i)
    {
        pFileInfo = (FileInfo*)(pdu -> caMsg) + i;
        QListWidgetItem *pItem = new QListWidgetItem;
        if(pFileInfo ->bIsDir)
        {
            pItem -> setIcon(QIcon(QPixmap(":/icon/dir.jpeg")));
            pItem -> setText(QString("%1\t%2KB\t%3").arg(pFileInfo->caName)
                            .arg(QString::number(int(pFileInfo->uiSize / 1024))).arg(pFileInfo->caTime));// 文件名 文件大小 最后修改时间  形式展示文件
            dirItem.append(pItem);
        }
        else
        {
            pItem -> setIcon(QIcon(QPixmap(":/icon/file.jpeg")));
            pItem ->setText(QString("%1\t%2KB\t%3").arg(pFileInfo->caName)
                            .arg(QString::number(int(pFileInfo->uiSize / 1024))).arg(pFileInfo->caTime));
            fileItem.append(pItem);
        }
    }
    for(auto it : dirItem)
        m_pFileListW->addItem(it);
    for(auto it : fileItem)
        m_pFileListW->addItem(it);
}

void FileSystem::createDir()
{
    QString strCreateDirName = QInputDialog::getText(this, "新建文件夹", "文件夹名：");
    QString strCurPath = TcpClient::getInstance().getStrCurPath();
    if(!strCreateDirName.isEmpty())
    {
        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu -> uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
        strncpy(pdu -> caData, strCreateDirName.toStdString().c_str(), strCreateDirName.size());
        memcpy((char*)pdu ->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
    else
        QMessageBox::warning(this, "新建文件夹", "文件夹名字不能为空!!!");
}

void FileSystem::flushDir()
{
    QString strCurPath = TcpClient::getInstance().getStrCurPath();
    PDU *pdu = mkPDU(strCurPath.size() + 1);
    pdu -> uiMsgType = ENUM_MSG_TYPE_FLUSH_DIR_REQUEST;
    memcpy((char*)pdu ->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void FileSystem::delFileOrDir()
{
    QString strCurPath = TcpClient::getInstance().getStrCurPath();
    QListWidgetItem *qItem = m_pFileListW->currentItem();
    if(nullptr == qItem)
    {
        QMessageBox::warning(this, "删除文件", "请选中需要删除的文件");
        return ;
    }
    QString strFileName = qItem->text().split('\t')[0];
    QString strDelPath = QString("%1/%2").arg(strCurPath).arg(QString(strFileName));

    QByteArray waitDelFileOrDirPath = strDelPath.toUtf8();

    PDU *pdu = mkPDU(waitDelFileOrDirPath.size() + 1);
    pdu -> uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_REQUEST;
    memcpy((char*)pdu -> caData, strFileName.toUtf8().toStdString().c_str(), strFileName.size());
    memcpy((char*)pdu ->caMsg, waitDelFileOrDirPath.toStdString().c_str(), waitDelFileOrDirPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void FileSystem::renameFile()
{
    QString strCurPath = TcpClient::getInstance().getStrCurPath();
    QListWidgetItem *qItem = m_pFileListW->currentItem();
    if(nullptr == qItem)
    {
        QMessageBox::warning(this, "重命名文件", "请选择需要重命名的文件！");
        return ;
    }
    QString strOldName = qItem -> text().split('\t')[0];
    QString strNewName = QInputDialog::getText(this, "文件重命名", "新文件名：");

    if(strNewName.isEmpty())
    {
        QMessageBox::warning(this, "重命名文件", "文件名不能为空！");
        return ;
    }

    PDU *pdu = mkPDU(strCurPath.size() + 1);

    pdu -> uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;

    strncpy(pdu -> caData, strOldName.toStdString().c_str(), 32);
    strncpy(pdu -> caData + 32, strNewName.toStdString().c_str(), 32);
    memcpy((char*)pdu ->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
    free(pdu);
    pdu = NULL;
}

void FileSystem::entryDir(const QModelIndex &index)
{
    QString strCurPath = TcpClient::getInstance().getStrCurPath();
    QString strFileName = index.data().toString();
    strFileName = strFileName.split('\t')[0];
    QByteArray strEntryPath = QString("%1/%2").arg(strCurPath).arg(strFileName).toUtf8();

    m_strTryEntryDir = strEntryPath;
    PDU* pdu = mkPDU(strEntryPath.size() + 1);

    pdu -> uiMsgType = ENUM_MSG_TYPE_ENTRY_DIR_REQUEST;
    memcpy((char*)pdu -> caMsg, strEntryPath.toStdString().c_str(), strEntryPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void FileSystem::returnPreDir()
{
    QString strCurPath = TcpClient::getInstance().getStrCurPath();
    QString strRootPath = TcpClient::getInstance().getStrRootPath();

    if(strCurPath == strRootPath)
    {
        QMessageBox::warning(this, "返回上一目录", "已经是根目录！");
        return ;
    }
    int index = strCurPath.lastIndexOf("/");
    strCurPath = strCurPath.remove(index, strCurPath.size() - index);

    m_strTryEntryDir = strCurPath;

    PDU* pdu = mkPDU(strCurPath.size() + 1);
    pdu -> uiMsgType = ENUM_MSG_TYPE_PRE_DIR_REQUEST;
    memcpy((char*)pdu -> caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void FileSystem::uploadFile()
{
    QString strCurPath = TcpClient::getInstance().getStrCurPath();
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    if(m_strUploadFilePath.isEmpty())
    {
        QMessageBox::warning(this, "上传文件", "请选择需要上传的文件！");
        return ;
    }
    int index = m_strUploadFilePath.lastIndexOf('/');
    QString strFileName = m_strUploadFilePath.right(m_strUploadFilePath.size() - index - 1);
    uploadFileProgress::getProgress().setFileNameToLable("文件上传:" + strFileName);

    QFile file(m_strUploadFilePath);
    qint64 fileSize = file.size();

    PDU* pdu = mkPDU(strCurPath.size() + 1);
    pdu -> uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
    memcpy(pdu -> caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    sprintf(pdu -> caData, "%s %lld", strFileName.toStdString().c_str(), fileSize);
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void FileSystem::startTimer()
{
    m_pTimer -> start(1000); // 1000ms
}

void FileSystem::uploadFileData()
{
    m_pTimer->stop();
    if(uploadFileProgress::getProgress().isHidden())
        uploadFileProgress::getProgress().show();
    QFile file(m_strUploadFilePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "打开文件", "打开文件失败！");
        return;
    }
    char *pBuffer = new char[4096];
    qint64 iActualSize = 0;
    qint64 nFileSize = file.size();
    qint64 nUploadFileSize = 0;
    uploadFileProgress::getProgress().setProgressValue(nUploadFileSize / nFileSize);
    while(true)
    {
        iActualSize = file.read(pBuffer, 4096);
        nUploadFileSize += iActualSize;
        uploadFileProgress::getProgress().setProgressValue(((qreal)nUploadFileSize / (qreal)nFileSize) * 100);
        if (iActualSize > 0 && iActualSize <= 4096)
        {
            AES aes;
            QByteArray encryptText = aes.scanfData(QByteArray(const_cast<char*>(pBuffer), iActualSize));
            PDU *resPdu = mkPDU(encryptText.length());
            resPdu -> uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_BYTE_TIPS;
            int fillByteNum = padding::getPKCS7PaddedLength(iActualSize, 16);
            int isFillByte = (0 == fillByteNum) ? 0 : 1;
            sprintf(resPdu -> caData, "%d %d", isFillByte, iActualSize);
            memcpy((char*)resPdu -> caMsg, encryptText.data(), encryptText.length());
            TcpClient::getInstance().getTcpSocket().write((char*)resPdu, resPdu -> uiPDULen);
            SYNCED_EXEC_DELAY_TIME(10);
            pBuffer[4096] = {'\0'};
            free(resPdu);
            resPdu = nullptr;
        }
        else if (iActualSize == 0)
        {
            SYNCED_EXEC_DELAY_TIME(10);
            char emptyPackage[10] = {'e', 'm', 'p', 't', 'y', 'p', 'a', 'c', 'k', '\0'};
            TcpClient::getInstance().getTcpSocket().write(emptyPackage, 10);
            uploadFileProgress::getProgress().hide();
            break;
        }
        else
        {
            QMessageBox::warning(this, "上传文件", "上传失败！");
            break;
        }
    }
    file.close();
    delete [] pBuffer;
    pBuffer = nullptr;
    m_strUploadFilePath.clear();
}

void FileSystem::downloadFile()
{
    QListWidgetItem *pItem = m_pFileListW->currentItem();
    if(nullptr == pItem)
    {
        QMessageBox::warning(this, "下载文件", "请选择要下载的文件！");
        return ;
    }

    QString strDownloadFilePath = QFileDialog::getSaveFileName();
    if(strDownloadFilePath.isEmpty())
    {
        QMessageBox::warning(this, "下载文件", "请指定下载文件的位置！");
        m_downloadFile->file.setFileName("");
        return ;
    }

    m_downloadFile->file.setFileName(strDownloadFilePath);

    QByteArray strCurPath = TcpClient::getInstance().getStrCurPath().toUtf8();
    QByteArray strFileName = pItem->text().split('\t')[0].toUtf8();
    uploadFileProgress::getProgress().setFileNameToLable("文件下载:" + strFileName);
    m_downloadFileName = strFileName;
    PDU* pdu = mkPDU(strCurPath.size() + 1);
    pdu -> uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
    memcpy((char*)pdu -> caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    strncpy(pdu -> caData, strFileName.toStdString().c_str(), strFileName.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);

    free(pdu);
    pdu = nullptr;
}

void FileSystem::moveFile()
{
    QListWidgetItem *pItem = m_pFileListW->currentItem();
    if(pItem == nullptr)
    {
        QMessageBox::warning(this, "移动文件", "请选择需要移动的文件！");
        return ;
    }

    m_strMoveFileName = pItem -> text().split('\t')[0];
    m_strMoveOldDir = TcpClient::getInstance().getStrCurPath();

    m_pMoveDesDirDB->setEnabled(true);
    QMessageBox::information(this, "移动文件", "请跳转到需要移动到的目录，\n然后点击“目标目录”按钮。");
}

void FileSystem::moveDesDir()
{
    QString strDesDir = TcpClient::getInstance().getStrCurPath();
    QMessageBox::StandardButton sbMoveAffirm;
    QString strMoveAffirm = QString("您确认将 %1 的 %2 文件\n移动到 %3 目录下吗？")
            .arg(m_strMoveOldDir).arg(m_strMoveFileName).arg(strDesDir);
    sbMoveAffirm = QMessageBox::question(this, "移动文件", strMoveAffirm);
    if(sbMoveAffirm == QMessageBox::No)
    {
        m_strMoveOldDir.clear();
        m_strMoveFileName.clear();
        m_pMoveDesDirDB->setEnabled(false);
        return ;
    }

    PDU *pdu = mkPDU(strDesDir.size() + m_strMoveOldDir.size() + 5);

    pdu -> uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
    sprintf((char*)pdu -> caMsg, "%s %s", strDesDir.toStdString().c_str(),
            m_strMoveOldDir.toStdString().c_str());
    sprintf(pdu -> caData, "%s %d %d", m_strMoveFileName.toStdString().c_str(), strDesDir.size(), m_strMoveOldDir.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
    free(pdu);
    pdu = nullptr;

    m_strMoveOldDir.clear();
    m_strMoveFileName.clear();
    m_pMoveDesDirDB->setEnabled(false);
}

void FileSystem::shareFile()
{

    QListWidgetItem *pFileItem = m_pFileListW->currentItem();
    if(nullptr == pFileItem)
    {
        QMessageBox::warning(this, "分享文件", "请选择要分享的文件！");
        return ;
    }
    m_strSharedFileName = pFileItem->text().split('\t')[0];
    m_strSharedFilePath = QString("%1/%2").arg(TcpClient::getInstance().getStrCurPath()).arg(m_strSharedFileName);

    QListWidget *friendLW = OperateWidget::getInstance().getPFriend()->getPFriendLW();

    m_pSharedFileFLW->updateFriendList(friendLW);
    if(m_pSharedFileFLW->isHidden() || m_pSharedFileFLW -> isMinimized())
    {
        m_pSharedFileFLW->showNormal();
    }

}

TransFile *FileSystem::getDownloadFileInfo()
{
    return m_downloadFile;
}

QString FileSystem::getStrSharedFileName() const
{
    return m_strSharedFileName;
}

void FileSystem::operateFileOrDirUpFileListWidget()
{
    QString strCurPath = TcpClient::getInstance().getStrCurPath();
    PDU *pdu = mkPDU(strCurPath.size() + 1);
    pdu -> uiMsgType = ENUM_MSG_TYPE_FLUSH_DIR_REQUEST;
    memcpy((char*)pdu ->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
    free(pdu);
    pdu = nullptr;
}

QTimer *FileSystem::getTimer()
{
    return m_pTimer;
}

void FileSystem::clearUploadFilePath()
{
    m_strUploadFilePath.clear();
}

QString FileSystem::getUploadFilePath()
{
    return m_strUploadFilePath;
}

QString FileSystem::getDownloadFileName()
{
    return m_downloadFileName;
}

QString FileSystem::getStrSharedFilePath() const
{
    return m_strSharedFilePath;
}

QString FileSystem::strTryEntryDir() const
{
    return m_strTryEntryDir;
}

void FileSystem::setStrTryEntryDir(const QString &strTryEntryDir)
{
    m_strTryEntryDir = strTryEntryDir;
}

void FileSystem::clearStrTryEntryDir()
{
    m_strTryEntryDir.clear();
}
