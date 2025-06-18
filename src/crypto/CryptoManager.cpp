#include "CryptoManager.h"
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QDataStream>
#include <QBuffer>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QRandomGenerator>

// 静态成员初始化
CryptoManager* CryptoManager::s_instance = nullptr;

// 加密常量
static const int SALT_SIZE = 32;
static const int KEY_SIZE = 32;
static const int IV_SIZE = 16;
static const int ITERATIONS = 100000;

/**
 * @brief 获取加密管理器的单例实例
 * @return 加密管理器指针
 */
CryptoManager* CryptoManager::instance()
{
    if (!s_instance) {
        s_instance = new CryptoManager();
    }
    return s_instance;
}

/**
 * @brief 私有构造函数
 * @param parent 父对象指针
 */
CryptoManager::CryptoManager(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_settings(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/crypto.ini", QSettings::IniFormat)
{
}

/**
 * @brief 析构函数
 */
CryptoManager::~CryptoManager()
{
    clear();
}

/**
 * @brief 初始化加密管理器
 * @param masterPassword 主密码，用于生成加密密钥
 * @return 初始化是否成功
 */
bool CryptoManager::initialize(const QString &masterPassword)
{
    if (masterPassword.isEmpty()) {
        emit cryptoError("Master password cannot be empty");
        return false;
    }

    // 加载或生成盐值
    if (!loadSalt()) {
        m_salt = generateSalt();
        saveSalt();
    }

    // 从主密码生成加密密钥
    m_encryptionKey = deriveKey(masterPassword);
    
    if (m_encryptionKey.isEmpty()) {
        emit cryptoError("Failed to derive encryption key");
        return false;
    }

    m_initialized = true;
    qInfo() << "CryptoManager initialized successfully";
    return true;
}

/**
 * @brief 检查是否已初始化
 * @return 如果已初始化则返回true
 */
bool CryptoManager::isInitialized() const
{
    return m_initialized;
}

/**
 * @brief 加密字符串
 * @param plaintext 明文字符串
 * @return 加密后的Base64编码字符串，失败返回空字符串
 */
QString CryptoManager::encryptString(const QString &plaintext)
{
    if (!m_initialized) {
        emit cryptoError("CryptoManager not initialized");
        return QString();
    }

    if (plaintext.isEmpty()) {
        return QString();
    }

    try {
        // 生成随机IV
        QByteArray iv = generateRandomBytes(IV_SIZE);
        
        // 将明文转换为字节数组
        QByteArray plaintextBytes = plaintext.toUtf8();
        
        // 创建加密数据流
        QByteArray encryptedData;
        QBuffer buffer(&encryptedData);
        buffer.open(QIODevice::WriteOnly);
        
        // 写入IV
        buffer.write(iv);
        
        // 简单的XOR加密（在实际应用中应该使用更安全的加密算法）
        // 这里使用简单的异或加密作为示例
        for (int i = 0; i < plaintextBytes.size(); ++i) {
            char encryptedByte = plaintextBytes[i] ^ m_encryptionKey[i % m_encryptionKey.size()];
            buffer.write(&encryptedByte, 1);
        }
        
        buffer.close();
        
        // 返回Base64编码的加密数据
        return encryptedData.toBase64();
        
    } catch (const std::exception &e) {
        QString error = QString("Encryption failed: %1").arg(e.what());
        emit cryptoError(error);
        return QString();
    }
}

/**
 * @brief 解密字符串
 * @param ciphertext 加密的Base64编码字符串
 * @return 解密后的明文字符串，失败返回空字符串
 */
QString CryptoManager::decryptString(const QString &ciphertext)
{
    if (!m_initialized) {
        emit cryptoError("CryptoManager not initialized");
        return QString();
    }

    if (ciphertext.isEmpty()) {
        return QString();
    }

    try {
        // 解码Base64数据
        QByteArray encryptedData = QByteArray::fromBase64(ciphertext.toUtf8());
        
        if (encryptedData.size() < IV_SIZE) {
            emit cryptoError("Invalid encrypted data");
            return QString();
        }
        
        // 读取IV
        QByteArray iv = encryptedData.left(IV_SIZE);
        QByteArray ciphertextBytes = encryptedData.mid(IV_SIZE);
        
        // 创建解密数据流
        QByteArray decryptedData;
        QBuffer buffer(&ciphertextBytes);
        buffer.open(QIODevice::ReadOnly);
        
        // 简单的XOR解密
        for (int i = 0; i < ciphertextBytes.size(); ++i) {
            char decryptedByte = ciphertextBytes[i] ^ m_encryptionKey[i % m_encryptionKey.size()];
            decryptedData.append(decryptedByte);
        }
        
        buffer.close();
        
        // 返回解密后的字符串
        return QString::fromUtf8(decryptedData);
        
    } catch (const std::exception &e) {
        QString error = QString("Decryption failed: %1").arg(e.what());
        emit cryptoError(error);
        return QString();
    }
}

/**
 * @brief 验证主密码
 * @param masterPassword 要验证的主密码
 * @return 密码是否正确
 */
bool CryptoManager::verifyMasterPassword(const QString &masterPassword)
{
    if (masterPassword.isEmpty()) {
        return false;
    }

    // 从设置中读取测试数据
    QString testData = m_settings.value("test_data").toString();
    if (testData.isEmpty()) {
        // 如果没有测试数据，创建一个
        QString testString = "test_verification_string";
        QByteArray testKey = deriveKey(masterPassword);
        
        // 临时加密测试字符串
        QByteArray tempKey = m_encryptionKey;
        m_encryptionKey = testKey;
        
        QString encrypted = encryptString(testString);
        m_settings.setValue("test_data", encrypted);
        
        m_encryptionKey = tempKey;
        return true;
    }

    // 尝试解密测试数据
    QByteArray testKey = deriveKey(masterPassword);
    QByteArray tempKey = m_encryptionKey;
    m_encryptionKey = testKey;
    
    QString decrypted = decryptString(testData);
    m_encryptionKey = tempKey;
    
    return decrypted == "test_verification_string";
}

/**
 * @brief 更改主密码
 * @param oldPassword 旧主密码
 * @param newPassword 新主密码
 * @return 更改是否成功
 */
bool CryptoManager::changeMasterPassword(const QString &oldPassword, const QString &newPassword)
{
    if (!verifyMasterPassword(oldPassword)) {
        emit cryptoError("Old password is incorrect");
        return false;
    }

    if (newPassword.isEmpty()) {
        emit cryptoError("New password cannot be empty");
        return false;
    }

    // 生成新的盐值和密钥
    m_salt = generateSalt();
    m_encryptionKey = deriveKey(newPassword);
    
    // 保存新的盐值
    saveSalt();
    
    // 更新测试数据
    QString testString = "test_verification_string";
    QString encrypted = encryptString(testString);
    m_settings.setValue("test_data", encrypted);
    
    qInfo() << "Master password changed successfully";
    return true;
}

/**
 * @brief 清除加密状态
 */
void CryptoManager::clear()
{
    m_encryptionKey.clear();
    m_salt.clear();
    m_initialized = false;
}

/**
 * @brief 从主密码生成加密密钥
 * @param masterPassword 主密码
 * @return 生成的密钥
 */
QByteArray CryptoManager::deriveKey(const QString &masterPassword)
{
    // 使用PBKDF2算法从主密码和盐值生成密钥
    QByteArray passwordBytes = masterPassword.toUtf8();
    QByteArray key = QCryptographicHash::hash(passwordBytes + m_salt, QCryptographicHash::Sha256);
    
    // 多次迭代以增加安全性
    for (int i = 1; i < ITERATIONS; ++i) {
        key = QCryptographicHash::hash(key + passwordBytes + m_salt, QCryptographicHash::Sha256);
    }
    
    return key;
}

/**
 * @brief 生成随机盐值
 * @return 随机盐值
 */
QByteArray CryptoManager::generateSalt()
{
    return generateRandomBytes(SALT_SIZE);
}

/**
 * @brief 保存盐值到设置
 */
void CryptoManager::saveSalt()
{
    m_settings.setValue("salt", m_salt.toBase64());
}

/**
 * @brief 从设置加载盐值
 * @return 加载是否成功
 */
bool CryptoManager::loadSalt()
{
    QString saltString = m_settings.value("salt").toString();
    if (saltString.isEmpty()) {
        return false;
    }
    
    m_salt = QByteArray::fromBase64(saltString.toUtf8());
    return m_salt.size() == SALT_SIZE;
}

/**
 * @brief 生成随机字节
 * @param length 字节长度
 * @return 随机字节数组
 */
QByteArray CryptoManager::generateRandomBytes(int length)
{
    QByteArray randomBytes;
    randomBytes.resize(length);
    
    // 使用Qt6的随机数生成器
    QRandomGenerator *generator = QRandomGenerator::global();
    for (int i = 0; i < length; ++i) {
        randomBytes[i] = static_cast<char>(generator->bounded(256));
    }
    
    return randomBytes;
} 