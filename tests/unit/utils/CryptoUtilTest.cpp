#include "CryptoUtilTest.h"
#include <QSet>

using namespace ResticGUI::Utils;

namespace ResticGUI {
namespace Test {

// ========== AES-256 加密/解密测试 ==========

void CryptoUtilTest::testEncryptDecrypt()
{
    // 测试基本的加密解密功能
    QString plaintext = "Hello, Restic GUI!";
    QString key = "MySecretKey123";

    QString ciphertext = CryptoUtil::encrypt(plaintext, key);

    // 密文不应该等于明文
    QVERIFY(ciphertext != plaintext);
    QVERIFY(!ciphertext.isEmpty());

    // 解密应该得到原始明文
    QString decrypted = CryptoUtil::decrypt(ciphertext, key);
    QCOMPARE(decrypted, plaintext);
}

void CryptoUtilTest::testEncryptDecryptEmptyString()
{
    // 测试空字符串的加密解密
    QString plaintext = "";
    QString key = "MySecretKey123";

    QString ciphertext = CryptoUtil::encrypt(plaintext, key);
    QString decrypted = CryptoUtil::decrypt(ciphertext, key);

    QCOMPARE(decrypted, plaintext);
}

void CryptoUtilTest::testEncryptDecryptWithDifferentKeys()
{
    // 测试相同明文、不同密钥生成不同密文
    QString plaintext = "Test Data";
    QString key1 = "Key1";
    QString key2 = "Key2";

    QString ciphertext1 = CryptoUtil::encrypt(plaintext, key1);
    QString ciphertext2 = CryptoUtil::encrypt(plaintext, key2);

    // 不同密钥应该生成不同的密文
    QVERIFY(ciphertext1 != ciphertext2);

    // 各自用对应密钥解密应该都能得到原文
    QCOMPARE(CryptoUtil::decrypt(ciphertext1, key1), plaintext);
    QCOMPARE(CryptoUtil::decrypt(ciphertext2, key2), plaintext);
}

void CryptoUtilTest::testDecryptWithWrongKey()
{
    // 测试使用错误的密钥解密
    QString plaintext = "Sensitive Data";
    QString correctKey = "CorrectKey";
    QString wrongKey = "WrongKey";

    QString ciphertext = CryptoUtil::encrypt(plaintext, correctKey);
    QString decrypted = CryptoUtil::decrypt(ciphertext, wrongKey);

    // 使用错误密钥解密不应该得到原始明文
    QVERIFY(decrypted != plaintext);
}

// ========== SHA-256 哈希测试 ==========

void CryptoUtilTest::testSha256()
{
    // 测试 SHA-256 哈希功能
    QString data = "Test Data";
    QString hash = CryptoUtil::sha256(data);

    // 哈希值不应该为空
    QVERIFY(!hash.isEmpty());

    // SHA-256 哈希长度应该是 64 个字符（256位 = 32字节 = 64个十六进制字符）
    QCOMPARE(hash.length(), 64);

    // 哈希值应该是十六进制字符串（只包含 0-9, a-f）
    QRegExp hexRegex("^[0-9a-f]{64}$");
    QVERIFY(hexRegex.exactMatch(hash));
}

void CryptoUtilTest::testSha256EmptyString()
{
    // 测试空字符串的哈希
    QString hash = CryptoUtil::sha256("");

    QVERIFY(!hash.isEmpty());
    QCOMPARE(hash.length(), 64);

    // 空字符串的 SHA-256 哈希是固定的
    // e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
    QString emptyStrHash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    QCOMPARE(hash, emptyStrHash);
}

void CryptoUtilTest::testSha256Consistency()
{
    // 测试相同输入应该产生相同哈希
    QString data = "Consistent Data";

    QString hash1 = CryptoUtil::sha256(data);
    QString hash2 = CryptoUtil::sha256(data);

    QCOMPARE(hash1, hash2);

    // 不同输入应该产生不同哈希
    QString hash3 = CryptoUtil::sha256(data + " ");
    QVERIFY(hash1 != hash3);
}

// ========== 随机密钥生成测试 ==========

void CryptoUtilTest::testGenerateRandomKey()
{
    // 测试随机密钥生成
    QString key = CryptoUtil::generateRandomKey();

    QVERIFY(!key.isEmpty());
}

void CryptoUtilTest::testGenerateRandomKeyLength()
{
    // 测试指定长度的随机密钥
    int length1 = 16;
    int length2 = 32;
    int length3 = 64;

    QString key1 = CryptoUtil::generateRandomKey(length1);
    QString key2 = CryptoUtil::generateRandomKey(length2);
    QString key3 = CryptoUtil::generateRandomKey(length3);

    QCOMPARE(key1.length(), length1);
    QCOMPARE(key2.length(), length2);
    QCOMPARE(key3.length(), length3);
}

void CryptoUtilTest::testRandomKeyUniqueness()
{
    // 测试随机密钥的唯一性（生成多个密钥应该不同）
    QSet<QString> keys;
    int count = 100;

    for (int i = 0; i < count; ++i) {
        QString key = CryptoUtil::generateRandomKey(32);
        keys.insert(key);
    }

    // 所有生成的密钥应该都是唯一的
    QCOMPARE(keys.size(), count);
}

// ========== 盐值生成测试 ==========

void CryptoUtilTest::testGenerateSalt()
{
    // 测试盐值生成
    QByteArray salt = CryptoUtil::generateSalt();

    QVERIFY(!salt.isEmpty());
}

void CryptoUtilTest::testGenerateSaltLength()
{
    // 测试指定长度的盐值
    int length1 = 8;
    int length2 = 16;
    int length3 = 32;

    QByteArray salt1 = CryptoUtil::generateSalt(length1);
    QByteArray salt2 = CryptoUtil::generateSalt(length2);
    QByteArray salt3 = CryptoUtil::generateSalt(length3);

    QCOMPARE(salt1.length(), length1);
    QCOMPARE(salt2.length(), length2);
    QCOMPARE(salt3.length(), length3);
}

void CryptoUtilTest::testSaltUniqueness()
{
    // 测试盐值的唯一性
    QSet<QByteArray> salts;
    int count = 100;

    for (int i = 0; i < count; ++i) {
        QByteArray salt = CryptoUtil::generateSalt(16);
        salts.insert(salt);
    }

    // 所有生成的盐值应该都是唯一的
    QCOMPARE(salts.size(), count);
}

// ========== Base64 编码/解码测试 ==========

void CryptoUtilTest::testBase64EncodeDecode()
{
    // 测试 Base64 编码解码
    QByteArray originalData = "Hello, Restic GUI!";

    QString encoded = CryptoUtil::toBase64(originalData);
    QVERIFY(!encoded.isEmpty());

    QByteArray decoded = CryptoUtil::fromBase64(encoded);
    QCOMPARE(decoded, originalData);
}

void CryptoUtilTest::testBase64EncodeDecodeEmptyData()
{
    // 测试空数据的 Base64 编码解码
    QByteArray emptyData;

    QString encoded = CryptoUtil::toBase64(emptyData);
    QByteArray decoded = CryptoUtil::fromBase64(encoded);

    QCOMPARE(decoded, emptyData);
}

void CryptoUtilTest::testBase64EncodeBinaryData()
{
    // 测试二进制数据的 Base64 编码解码
    QByteArray binaryData;
    for (int i = 0; i < 256; ++i) {
        binaryData.append(static_cast<char>(i));
    }

    QString encoded = CryptoUtil::toBase64(binaryData);
    QByteArray decoded = CryptoUtil::fromBase64(encoded);

    QCOMPARE(decoded, binaryData);
}

} // namespace Test
} // namespace ResticGUI
