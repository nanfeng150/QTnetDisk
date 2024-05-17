#ifndef PADDING_H
#define PADDING_H
#include <QByteArray>

class padding
{
public:
    padding();
    static int getPKCS7PaddedLength(qint64 dataLen, int alignSize);
    static QByteArray PKCS7Padding(const QByteArray &in, int alignSize);
    static QByteArray PKCS7UnPadding(const QByteArray &in);
};

#endif // PADDING_H

