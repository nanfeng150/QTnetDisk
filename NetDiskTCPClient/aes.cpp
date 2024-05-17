#include "aes.h"
#include <openssl/modes.h>
#include <openssl/aes.h>


AES::AES(){}

AES::~AES(){}

/**
 * @brief AES::ecb_encrypt
 * ECB模式加解密，填充模式采用PKCS7Padding，
 * 支持对任意长度明文进行加解密。
 * @param in 输入数据
 * @param out 输出结果
 * @param key 密钥，长度必须是16/24/32字节，否则加密失败
 * @param enc true-加密，false-解密
 * @return 执行结果
 */
bool AES::ecb_encrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, bool enc)
{
    // 检查密钥合法性(只能是16、24、32字节)
    Q_ASSERT(key.size() == 16 || key.size() == 24 || key.size() == 32);

    if (enc)
    {
        // 生成加密key
        AES_KEY aes_key;
        if (AES_set_encrypt_key((const unsigned char*)key.data(), key.size() * 8, &aes_key) != 0)
        {
            return false;
        }

        // 进行PKCS7Padding填充
        QByteArray inTemp = padding::PKCS7Padding(in, AES_BLOCK_SIZE);

        // 执行ECB模式加密
        out.resize(inTemp.size()); // 调整输出buf大小
        for (int i = 0; i < inTemp.size() / AES_BLOCK_SIZE; i++)
        {
            AES_ecb_encrypt((const unsigned char*)inTemp.data() + AES_BLOCK_SIZE * i,
                            (unsigned char*)out.data() + AES_BLOCK_SIZE * i,
                            &aes_key,
                            AES_ENCRYPT);
        }
        return true;
    }
    else
    {
        // 生成解密key
        AES_KEY aes_key;
        if (AES_set_decrypt_key((const unsigned char*)key.data(), key.size() * 8, &aes_key) != 0)
        {
            return false;
        }

        // 执行ECB模式解密
        out.resize(in.size()); // 调整输出buf大小
        for (int i = 0; i < in.size() / AES_BLOCK_SIZE; i++)
        {
            AES_ecb_encrypt((const unsigned char*)in.data() + AES_BLOCK_SIZE * i,
                            (unsigned char*)out.data() + AES_BLOCK_SIZE * i,
                            &aes_key,
                            AES_DECRYPT);
        }

        // 解除PKCS7Padding填充
        out = padding::PKCS7UnPadding(out);
        return true;
    }
}

QByteArray AES::scanfData(QByteArray plainText)
{
    QByteArray encryptText;
    encryptText.clear();
    QByteArray key = QByteArray::fromHex("8cc72b05705d5c46f412af8cbed55aad");
    AES aes;
    aes.ecb_encrypt(plainText, encryptText, key, ENUM_AES_ENCRYPT);     // 加密
    return encryptText;
}

QByteArray AES::printfData(QByteArray encryptText)
{
    QByteArray decryptText;
    decryptText.clear();
    QByteArray key = QByteArray::fromHex("8cc72b05705d5c46f412af8cbed55aad");
    AES aes;
    aes.ecb_encrypt(encryptText, decryptText, key, ENUM_AES_DECRYPT);  // 解密
    return decryptText;
}
