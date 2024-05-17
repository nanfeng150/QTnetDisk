#ifndef UPLOADFILEPROGRESS_H
#define UPLOADFILEPROGRESS_H

#include <QWidget>

namespace Ui {
class uploadFileProgress;
}

class uploadFileProgress : public QWidget
{
    Q_OBJECT

public:
    explicit uploadFileProgress(QWidget *parent = nullptr);
    ~uploadFileProgress();

    static uploadFileProgress &getProgress();

    void setProgressValue(int value);

    void setFileNameToLable(QString fileName);

private:
    Ui::uploadFileProgress *ui;
};

#endif // UPLOADFILEPROGRESS_H
