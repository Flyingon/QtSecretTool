#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariant>
#include "crypto/CryptoManager.h"
#include "models/PasswordItem.h"
#include "models/PasswordListModel.h"
#include "database/DatabaseManager.h"

/**
 * @brief 密码管理器类
 * 
 * 作为QML和C++加密功能之间的桥梁
 * 提供主密码设置、验证和更改功能
 * 以及完整的密码管理功能
 */
class PasswordManager : public QObject
{
    Q_OBJECT

    // QML属性
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(int totalPasswordsCount READ totalPasswordsCount NOTIFY totalPasswordsCountChanged)
    Q_PROPERTY(PasswordListModel* passwordListModel READ passwordListModel CONSTANT)

public:
    explicit PasswordManager(QObject *parent = nullptr);

    // 属性访问方法
    bool isLoading() const { return m_isLoading; }
    QString lastError() const { return m_lastError; }
    int totalPasswordsCount() const;
    PasswordListModel* passwordListModel() const { return m_passwordListModel; }

    /**
     * @brief 检查是否已设置主密码
     * @return 如果已设置主密码则返回true
     */
    Q_INVOKABLE bool hasMasterPassword() const;

    /**
     * @brief 设置主密码
     * @param password 主密码
     * @return 设置是否成功
     */
    Q_INVOKABLE bool setMasterPassword(const QString &password);

    /**
     * @brief 验证主密码
     * @param password 要验证的主密码
     * @return 密码是否正确
     */
    Q_INVOKABLE bool verifyMasterPassword(const QString &password);

    /**
     * @brief 更改主密码
     * @param oldPassword 旧主密码
     * @param newPassword 新主密码
     * @return 更改是否成功
     */
    Q_INVOKABLE bool changeMasterPassword(const QString &oldPassword, const QString &newPassword);

    /**
     * @brief 初始化加密管理器
     * @param password 主密码
     * @return 初始化是否成功
     */
    Q_INVOKABLE bool initializeCrypto(const QString &password);

    /**
     * @brief 检查加密管理器是否已初始化
     * @return 如果已初始化则返回true
     */
    Q_INVOKABLE bool isCryptoInitialized() const;

    /**
     * @brief 初始化密码管理器
     * @return 初始化是否成功
     */
    Q_INVOKABLE bool initialize();

    /**
     * @brief 生成随机密码
     * @param length 密码长度
     * @param includeSymbols 是否包含特殊符号
     * @return 生成的密码
     */
    Q_INVOKABLE QString generatePassword(int length = 12, bool includeSymbols = true);

    // 密码管理方法
    /**
     * @brief 创建密码项目
     * @param title 标题
     * @param username 用户名
     * @param password 密码
     * @param website 网站
     * @param notes 备注
     * @param category 分类
     * @return 密码项目指针
     */
    Q_INVOKABLE PasswordItem* createPasswordItem(const QString &title, 
                                                const QString &username, 
                                                const QString &password,
                                                const QString &website = QString(),
                                                const QString &notes = QString(),
                                                const QString &category = QString());

    /**
     * @brief 保存密码
     * @param item 密码项目
     * @return 保存是否成功
     */
    Q_INVOKABLE bool savePassword(PasswordItem *item);

    /**
     * @brief 更新密码
     * @param item 密码项目
     * @return 更新是否成功
     */
    Q_INVOKABLE bool updatePassword(PasswordItem *item);

    /**
     * @brief 删除密码
     * @param id 密码ID
     * @return 删除是否成功
     */
    Q_INVOKABLE bool deletePassword(int id);

    /**
     * @brief 搜索密码
     * @param searchTerm 搜索词
     */
    Q_INVOKABLE void searchPasswords(const QString &searchTerm);

    /**
     * @brief 显示收藏夹
     * @param showOnly 是否只显示收藏
     */
    Q_INVOKABLE void showFavoritesOnly(bool showOnly);

    /**
     * @brief 清除过滤器
     */
    Q_INVOKABLE void clearFilters();

    /**
     * @brief 按分类过滤
     * @param category 分类名称
     */
    Q_INVOKABLE void filterByCategory(const QString &category);

    /**
     * @brief 获取所有分类
     * @return 分类列表
     */
    Q_INVOKABLE QStringList getCategories();

    /**
     * @brief 刷新密码列表
     */
    Q_INVOKABLE void refreshPasswordList();

    // 数据库管理方法
    /**
     * @brief 备份数据库
     * @param filePath 备份文件路径
     * @return 备份是否成功
     */
    Q_INVOKABLE bool backupDatabase(const QString &filePath);

    /**
     * @brief 恢复数据库
     * @param filePath 恢复文件路径
     * @return 恢复是否成功
     */
    Q_INVOKABLE bool restoreDatabase(const QString &filePath);

    /**
     * @brief 导出为JSON
     * @param filePath 导出文件路径
     * @param includePasswords 是否包含密码
     * @return 导出是否成功
     */
    Q_INVOKABLE bool exportToJson(const QString &filePath, bool includePasswords = false);

    /**
     * @brief 导出为CSV
     * @param filePath 导出文件路径
     * @param includePasswords 是否包含密码
     * @return 导出是否成功
     */
    Q_INVOKABLE bool exportToCsv(const QString &filePath, bool includePasswords = false);

    /**
     * @brief 从JSON导入
     * @param filePath 导入文件路径
     * @param overwrite 是否覆盖现有数据
     * @return 导入是否成功
     */
    Q_INVOKABLE bool importFromJson(const QString &filePath, bool overwrite = false);

    /**
     * @brief 从CSV导入
     * @param filePath 导入文件路径
     * @param overwrite 是否覆盖现有数据
     * @return 导入是否成功
     */
    Q_INVOKABLE bool importFromCsv(const QString &filePath, bool overwrite = false);

    /**
     * @brief 清除所有密码
     * @return 清除是否成功
     */
    Q_INVOKABLE bool clearAllPasswords();

signals:
    /**
     * @brief 主密码设置成功信号
     */
    void masterPasswordSet();

    /**
     * @brief 主密码验证成功信号
     */
    void masterPasswordVerified();

    /**
     * @brief 主密码更改成功信号
     */
    void masterPasswordChanged();

    /**
     * @brief 密码操作错误信号
     * @param error 错误信息
     */
    void passwordError(const QString &error);

    /**
     * @brief 加载状态改变信号
     */
    void isLoadingChanged();

    /**
     * @brief 最后错误改变信号
     */
    void lastErrorChanged();

    /**
     * @brief 密码总数改变信号
     */
    void totalPasswordsCountChanged();

private:
    CryptoManager *m_cryptoManager;
    DatabaseManager *m_databaseManager;
    PasswordListModel *m_passwordListModel;
    bool m_isLoading;
    QString m_lastError;

    void setLoading(bool loading);
    void setLastError(const QString &error);
    void clearLastError();
    
    /**
     * @brief 解析CSV行
     * @param line CSV行字符串
     * @return 字段列表
     */
    QStringList parseCsvLine(const QString &line);
};

#endif // PASSWORDMANAGER_H 