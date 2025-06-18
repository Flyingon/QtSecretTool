#include <QCoreApplication>
#include <QDebug>
#include <QSettings>
#include "src/crypto/CryptoManager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "Testing CryptoManager...";
    
    // 获取加密管理器实例
    CryptoManager *crypto = CryptoManager::instance();
    
    // 测试设置主密码
    QString masterPassword = "test123456";
    if (crypto->initialize(masterPassword)) {
        qDebug() << "CryptoManager initialized successfully";
    } else {
        qDebug() << "Failed to initialize CryptoManager";
        return -1;
    }
    
    // 测试加密和解密
    QString originalText = "Hello, this is a test password!";
    qDebug() << "Original text:" << originalText;
    
    QString encrypted = crypto->encryptString(originalText);
    qDebug() << "Encrypted:" << encrypted;
    
    QString decrypted = crypto->decryptString(encrypted);
    qDebug() << "Decrypted:" << decrypted;
    
    if (originalText == decrypted) {
        qDebug() << "✓ Encryption/Decryption test PASSED";
    } else {
        qDebug() << "✗ Encryption/Decryption test FAILED";
    }
    
    // 测试密码验证
    if (crypto->verifyMasterPassword(masterPassword)) {
        qDebug() << "✓ Password verification test PASSED";
    } else {
        qDebug() << "✗ Password verification test FAILED";
    }
    
    // 测试错误密码
    if (!crypto->verifyMasterPassword("wrongpassword")) {
        qDebug() << "✓ Wrong password test PASSED";
    } else {
        qDebug() << "✗ Wrong password test FAILED";
    }
    
    qDebug() << "All tests completed!";
    
    return 0;
} 