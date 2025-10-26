#ifndef CRYPTOUTILTEST_H
#define CRYPTOUTILTEST_H

#include "common/TestBase.h"
#include "utils/CryptoUtil.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief CryptoUtil 测试类
 *
 * 测试加密工具类的所有功能：
 * - AES-256 加密/解密
 * - SHA-256 哈希
 * - 随机密钥生成
 * - 盐值生成
 * - Base64 编码/解码
 */
class CryptoUtilTest : public TestBase
{
    Q_OBJECT

private slots:
    // AES-256 加密/解密测试
    void testEncryptDecrypt();
    void testEncryptDecryptEmptyString();
    void testEncryptDecryptWithDifferentKeys();
    void testDecryptWithWrongKey();

    // SHA-256 哈希测试
    void testSha256();
    void testSha256EmptyString();
    void testSha256Consistency();

    // 随机密钥生成测试
    void testGenerateRandomKey();
    void testGenerateRandomKeyLength();
    void testRandomKeyUniqueness();

    // 盐值生成测试
    void testGenerateSalt();
    void testGenerateSaltLength();
    void testSaltUniqueness();

    // Base64 编码/解码测试
    void testBase64EncodeDecode();
    void testBase64EncodeDecodeEmptyData();
    void testBase64EncodeBinaryData();
};

} // namespace Test
} // namespace ResticGUI

#endif // CRYPTOUTILTEST_H
