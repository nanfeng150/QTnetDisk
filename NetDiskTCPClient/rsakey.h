#ifndef RSAKEY_H
#define RSAKEY_H

#include <QApplication>
#include <QSslSocket>
#include <QNetworkAccessManager>
#include <QString>
#include <QDebug>
#include "openssl/ssl.h"
#include "openssl/evp.h"
#include "openssl/rand.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"
#include "openssl/err.h"
#include "openssl/aes.h"
#define BEGIN_RSA_PUBLIC_KEY "zhengchendong"

#define RSA_PUBLIC_KEY "-----BEGIN PUBLIC KEY-----\n"\
    "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCjBAChJDE8yhBSwsNmRzfDHoVo\n"\
    "FIFjWICaXfwUU5HGyYnW3AgKnX+itPfWNNXnD4fjOoQv+5VlRUvHgeuFB07yngEG\n"\
    "AH7WhCllPq1TvQWynQOqdx1Jx7H9w4YyW3IAASSHqQHzAMqSYy1zghcxjyI7NpXP\n"\
    "A7iMmZ8U0GhgUjcd9wIDAQAB\n"\
    "-----END PUBLIC KEY-----"

#define RSA_PRIVATE_KEY "-----BEGIN RSA PRIVATE KEY-----\n"\
    "MIICXAIBAAKBgQCjBAChJDE8yhBSwsNmRzfDHoVoFIFjWICaXfwUU5HGyYnW3AgK\n"\
    "nX+itPfWNNXnD4fjOoQv+5VlRUvHgeuFB07yngEGAH7WhCllPq1TvQWynQOqdx1J\n"\
    "x7H9w4YyW3IAASSHqQHzAMqSYy1zghcxjyI7NpXPA7iMmZ8U0GhgUjcd9wIDAQAB\n"\
    "AoGAf4kdGPUg1Gdd9/HKP9WOCchyJOiOBfRywNKw6hIrpbN9dKy2Wd+4wMoGb/7s\n"\
    "LpbXAG6Chqu0yQM68z5wzaXXG/fPvUOvNcAxKpnSeK/guzNETwxoSRe5duc2o+iY\n"\
    "m1qG7l6/tYsGZ/qzDHeNateMM4OpC4CobjOlKycpw7N6z6kCQQDQGOATh8B5UcG7\n"\
    "ViWZ37Yc0K64BK1MwaoS+R33BUWY+UAL5x/im5NmEHVwBSqThyKBReaZxAfGtA0i\n"\
    "1+AmITorAkEAyIp7N4gP7dbPmanR4dB/ca/y2b8YrFDKjolwceIJQPp/8zEPv28V\n"\
    "q7032u37UIytQI0CiwnPJw0gnFwBliMBZQJAKeG3x3Z88G3Z9eBCtRNnTalaYMLg\n"\
    "NiCMIEFvHHY9KaqQj1S3AB6breEGIcOIbCS4dxggbzYa8ozuT+Luw/3P/QJBAK5c\n"\
    "6aFw5hivizVaGCi09fpNN2Pn6XX5kQS5r1D1bYJXy2QRplWWKkt2a9gozzhL+F+F\n"\
    "u05Uqw6TNWV+AcseiJkCQE/LuxFAbIdmXLRDLZ9F8q6YB0sJJ/Xq0JqhHwusrEy9\n"\
    "sYzz9JVuLDZHDsmd8zlm593IVZYmz2Mj4Qqw+KZ0PF4=\n"\
    "-----END RSA PRIVATE KEY-----"

QString rsaPubEncrypt(const QString &strPlainData, const QString &strPubKey);//加密

QString rsaPriDecrypt(const QString &strDecryptData, const QString &strPriKey);//解密



#endif // RSAKEY_H
