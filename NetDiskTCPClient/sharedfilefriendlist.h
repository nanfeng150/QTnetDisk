#ifndef SHAREDFILEFRIENDLIST_H
#define SHAREDFILEFRIENDLIST_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QScrollArea>
#include <QCheckBox>
#include <QListWidget>

class sharedFileFriendList : public QWidget
{
    Q_OBJECT
public:
    explicit sharedFileFriendList(QWidget *parent = nullptr);
    void updateFriendList(QListWidget *pFriendList);

public slots:
    void selectAll();
    void cancleSelect();
    void affirmShare();
    void cancleShare();

signals:

private:
    QPushButton *m_pSelectAllPB = nullptr;
    QPushButton *m_pCancleSelectPB = nullptr;

    QPushButton *m_pAffirmPB = nullptr;
    QPushButton *m_pCanclePB = nullptr;

    QScrollArea *m_pFriendsSA = nullptr;
    QWidget *m_pFriendsWid = nullptr;
    QVBoxLayout *m_pFriendsVBL = nullptr;
    QButtonGroup *m_pFriendsBG = nullptr;
};

#endif // SHAREDFILEFRIENDLIST_H
