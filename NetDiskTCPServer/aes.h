#ifndef AES_H
#define AES_H

#include <QWidget>
#include <cstring>
#include <QDebug>
#include "openssl/aes.h"
#include "openssl/conf.h"
#include "openssl/evp.h"
#include "openssl/err.h"
#include <openssl/modes.h>
#include "padding.h"

namespace Ui {
class AES;
}

enum ENUM_AES{
    ENUM_AES_DECRYPT = 0,//解密
    ENUM_AES_ENCRYPT,    //加密
};

class AES
{
public:
    explicit AES();
    ~AES();

    bool ecb_encrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, bool enc);
    QByteArray scanfData(QByteArray plainText);
    QByteArray printfData(QByteArray encryptText);
private:

};

#endif // AES_H

