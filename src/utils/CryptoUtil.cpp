/**
 * @file CryptoUtil.cpp
 * @brief 加密工具类实现
 */

#include "CryptoUtil.h"
#include <QCryptographicHash>
#include <QRandomGenerator>

namespace ResticGUI {
namespace Utils {

QString CryptoUtil::encrypt(const QString& plaintext, const QString& key)
{
    // 简化实现：仅使用Base64编码
    // TODO: 实际项目应使用真正的AES-256加密（QCA库）
    QByteArray data = plaintext.toUtf8();
    return toBase64(data);
}

QString CryptoUtil::decrypt(const QString& ciphertext, const QString& key)
{
    // 简化实现：Base64解码
    // TODO: 实际项目应使用真正的AES-256解密
    QByteArray data = fromBase64(ciphertext);
    return QString::fromUtf8(data);
}

QString CryptoUtil::sha256(const QString& data)
{
    QByteArray hash = QCryptographicHash::hash(
        data.toUtf8(),
        QCryptographicHash::Sha256
    );
    return QString::fromLatin1(hash.toHex());
}

QString CryptoUtil::generateRandomKey(int length)
{
    QByteArray key;
    for (int i = 0; i < length; ++i) {
        key.append(QRandomGenerator::global()->bounded(256));
    }
    return toBase64(key);
}

QByteArray CryptoUtil::generateSalt(int length)
{
    QByteArray salt;
    for (int i = 0; i < length; ++i) {
        salt.append(QRandomGenerator::global()->bounded(256));
    }
    return salt;
}

QString CryptoUtil::toBase64(const QByteArray& data)
{
    return QString::fromLatin1(data.toBase64());
}

QByteArray CryptoUtil::fromBase64(const QString& data)
{
    return QByteArray::fromBase64(data.toLatin1());
}

} // namespace Utils
} // namespace ResticGUI
