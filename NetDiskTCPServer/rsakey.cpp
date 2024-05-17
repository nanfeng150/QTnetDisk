#include "rsakey.h"

QString rsaPubEncrypt(const QString &strPlainData, const QString &strPubKey)
{
    QByteArray pubKeyArry = strPubKey.toUtf8();
    uchar* pPubKey = (uchar*)pubKeyArry.data();
    BIO* pKeyBio = BIO_new_mem_buf(pPubKey, pubKeyArry.length());
    if (pKeyBio == nullptr) {
        return "";
    }

    RSA* pRsa = RSA_new();
    if (strPubKey.contains(BEGIN_RSA_PUBLIC_KEY)) {
        pRsa = PEM_read_bio_RSAPublicKey(pKeyBio, &pRsa, nullptr, nullptr);

    }
    else {
        pRsa = PEM_read_bio_RSA_PUBKEY(pKeyBio, &pRsa, nullptr, nullptr);

    }

    if (pRsa == nullptr) {
        BIO_free_all(pKeyBio);
        return "";
    }

    int nLen = RSA_size(pRsa);
    char* pEncryptBuf = new char[nLen];
    memset(pEncryptBuf, 0, nLen);

    //加密
    QByteArray plainDataArry = strPlainData.toUtf8();
    int nPlainDataLen = plainDataArry.length();
    uchar* pPlainData = (uchar*)plainDataArry.data();
    int nSize = RSA_public_encrypt(nPlainDataLen, pPlainData, (uchar*)pEncryptBuf, pRsa, RSA_PKCS1_PADDING);

    QString strEncryptData = "";
    if (nSize >= 0) {
        QByteArray arry(pEncryptBuf, nSize);
        strEncryptData = arry.toBase64();
    }

    delete []pEncryptBuf;
    pEncryptBuf = nullptr;
    BIO_free_all(pKeyBio);
    RSA_free(pRsa);

    return strEncryptData;
}

QString rsaPriDecrypt(const QString &strDecryptData, const QString &strPriKey)
{
    QByteArray priKeyArry = strPriKey.toUtf8();
    uchar* pPriKey = (uchar*)priKeyArry.data();
    BIO* pKeyBio = BIO_new_mem_buf(pPriKey, priKeyArry.length());
    if (pKeyBio == nullptr) {
        return "";
    }

    RSA* pRsa = RSA_new();
    pRsa = PEM_read_bio_RSAPrivateKey(pKeyBio, &pRsa, nullptr, nullptr);
    if (pRsa == nullptr) {
        BIO_free_all(pKeyBio);
        return "";
    }

    int nLen = RSA_size(pRsa);
    char* pPlainBuf = new char[nLen];
    memset(pPlainBuf, 0, nLen);

    //解密
    QByteArray decryptDataArry = strDecryptData.toUtf8();
    decryptDataArry = QByteArray::fromBase64(decryptDataArry);
    int nDecryptDataLen = decryptDataArry.length();
    uchar* pDecryptData = (uchar*)decryptDataArry.data();
    int nSize = RSA_private_decrypt(nDecryptDataLen, pDecryptData, (uchar*)pPlainBuf, pRsa, RSA_PKCS1_PADDING);

    QString strPlainData = "";
    if (nSize >= 0) {
        strPlainData = QByteArray(pPlainBuf, nSize);
    }

    delete []pPlainBuf;
    pPlainBuf = nullptr;
    BIO_free_all(pKeyBio);
    RSA_free(pRsa);

    return strPlainData;
}

