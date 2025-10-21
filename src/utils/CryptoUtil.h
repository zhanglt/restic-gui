/**
 * @file CryptoUtil.h
 * @brief 加密工具类
 */

#ifndef CRYPTOUTIL_H
#define CRYPTOUTIL_H

#include <QString>
#include <QByteArray>

namespace ResticGUI {
namespace Utils {

class CryptoUtil
{
public:
    // AES-256 加密/解密（简化版，实际应使用QCA或OpenSSL）
    static QString encrypt(const QString& plaintext, const QString& key);
    static QString decrypt(const QString& ciphertext, const QString& key);

    // SHA-256 哈希
    static QString sha256(const QString& data);

    // 生成随机数据
    static QString generateRandomKey(int length = 32);
    static QByteArray generateSalt(int length = 16);

    // Base64 编码/解码
    static QString toBase64(const QByteArray& data);
    static QByteArray fromBase64(const QString& data);

private:
    CryptoUtil() = delete;
};

} // namespace Utils
} // namespace ResticGUI

#endif // CRYPTOUTIL_H
