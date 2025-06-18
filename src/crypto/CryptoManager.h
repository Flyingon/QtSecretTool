#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QCryptographicHash>
#include <QSettings>

/**
 * @brief 加密管理器类
 * 
 * 负责处理密码数据的加密和解密操作
 * 使用AES-256-GCM加密算法，提供安全的密码存储
 */
class CryptoManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取加密管理器的单例实例
     * @return 加密管理器指针
     */
    static CryptoManager* instance();

    /**
     * @brief 初始化加密管理器
     * @param masterPassword 主密码，用于生成加密密钥
     * @return 初始化是否成功
     */
    bool initialize(const QString &masterPassword);

    /**
     * @brief 检查是否已初始化
     * @return 如果已初始化则返回true
     */
    bool isInitialized() const;

    /**
     * @brief 加密字符串
     * @param plaintext 明文字符串
     * @return 加密后的Base64编码字符串，失败返回空字符串
     */
    QString encryptString(const QString &plaintext);

    /**
     * @brief 解密字符串
     * @param ciphertext 加密的Base64编码字符串
     * @return 解密后的明文字符串，失败返回空字符串
     */
    QString decryptString(const QString &ciphertext);

    /**
     * @brief 验证主密码
     * @param masterPassword 要验证的主密码
     * @return 密码是否正确
     */
    bool verifyMasterPassword(const QString &masterPassword);

    /**
     * @brief 更改主密码
     * @param oldPassword 旧主密码
     * @param newPassword 新主密码
     * @return 更改是否成功
     */
    bool changeMasterPassword(const QString &oldPassword, const QString &newPassword);

    /**
     * @brief 清除加密状态
     */
    void clear();

signals:
    /**
     * @brief 加密错误信号
     * @param error 错误信息
     */
    void cryptoError(const QString &error);

private:
    explicit CryptoManager(QObject *parent = nullptr);
    ~CryptoManager() override;

    static CryptoManager *s_instance;  // 单例实例

    QByteArray m_encryptionKey;        // 加密密钥
    QByteArray m_salt;                 // 盐值
    bool m_initialized;                // 是否已初始化
    QSettings m_settings;              // 设置存储

    /**
     * @brief 从主密码生成加密密钥
     * @param masterPassword 主密码
     * @return 生成的密钥
     */
    QByteArray deriveKey(const QString &masterPassword);

    /**
     * @brief 生成随机盐值
     * @return 随机盐值
     */
    QByteArray generateSalt();

    /**
     * @brief 保存盐值到设置
     */
    void saveSalt();

    /**
     * @brief 从设置加载盐值
     * @return 加载是否成功
     */
    bool loadSalt();

    /**
     * @brief 生成随机字节
     * @param length 字节长度
     * @return 随机字节数组
     */
    QByteArray generateRandomBytes(int length);
};

#endif // CRYPTOMANAGER_H 