#include "uploadfileprogress.h"
#include "ui_uploadfileprogress.h"

uploadFileProgress::uploadFileProgress(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::uploadFileProgress)
{
    ui->setupUi(this);
}

uploadFileProgress::~uploadFileProgress()
{
    delete ui;
}

uploadFileProgress &uploadFileProgress::getProgress()
{
    static uploadFileProgress instance;
    return instance;
}

void uploadFileProgress::setProgressValue(int value)
{
    ui->upOrdownLoadPro->setValue(value);
}

void uploadFileProgress::setFileNameToLable(QString fileName)
{
    ui->fileOrDirName->setText(fileName);
}
