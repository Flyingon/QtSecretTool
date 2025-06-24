#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QList>
#include "models/PasswordItem.h"
#include "crypto/CryptoManager.h"

/**
 * @brief 数据库管理类
 * 
 * 负责管理SQLite数据库连接和所有数据库操作
 * 包括密码数据的增删改查、数据库初始化、备份等功能
 */
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取数据库管理器的单例实例
     * @return 数据库管理器指针
     */
    static DatabaseManager* instance();

    /**
     * @brief 初始化数据库连接
     * @param databasePath 数据库文件路径，如果为空则使用默认路径
     * @return 初始化是否成功
     */
    bool initialize(const QString &databasePath = QString());

    /**
     * @brief 设置数据库密码（SQLCipher）
     * @param password 主密码
     * @return 设置是否成功
     */
    bool setDatabasePassword(const QString &password);

    /**
     * @brief 验证数据库密码（SQLCipher）
     * @param password 主密码
     * @return 验证是否成功
     */
    bool verifyDatabasePassword(const QString &password);

    /**
     * @brief 更改数据库密码（SQLCipher）
     * @param oldPassword 旧密码
     * @param newPassword 新密码
     * @return 更改是否成功
     */
    bool changeDatabasePassword(const QString &oldPassword, const QString &newPassword);

    /**
     * @brief 检查数据库是否已加密
     * @return 如果数据库已加密则返回true
     */
    bool isDatabaseEncrypted() const;

    /**
     * @brief 关闭数据库连接
     */
    void closeDatabase();

    /**
     * @brief 检查数据库是否已连接
     * @return 如果数据库已连接则返回true
     */
    bool isConnected() const;

    // 密码项目数据库操作
    /**
     * @brief 保存密码项目到数据库
     * @param item 要保存的密码项目
     * @return 保存成功返回新的ID，失败返回-1
     */
    int savePasswordItem(PasswordItem *item);

    /**
     * @brief 更新数据库中的密码项目
     * @param item 要更新的密码项目
     * @return 更新是否成功
     */
    bool updatePasswordItem(PasswordItem *item);

    /**
     * @brief 从数据库删除密码项目
     * @param id 要删除的密码项目ID
     * @return 删除是否成功
     */
    bool deletePasswordItem(int id);

    /**
     * @brief 根据ID从数据库获取密码项目
     * @param id 密码项目ID
     * @return 密码项目指针，如果未找到则返回nullptr
     */
    PasswordItem* getPasswordItem(int id);

    /**
     * @brief 获取所有密码项目
     * @return 所有密码项目的列表
     */
    QList<PasswordItem*> getAllPasswordItems();

    /**
     * @brief 搜索密码项目
     * @param searchTerm 搜索词
     * @return 匹配的密码项目列表
     */
    QList<PasswordItem*> searchPasswordItems(const QString &searchTerm);

    /**
     * @brief 根据分类获取密码项目
     * @param category 分类名称
     * @return 指定分类的密码项目列表
     */
    QList<PasswordItem*> getPasswordItemsByCategory(const QString &category);

    /**
     * @brief 获取收藏的密码项目
     * @return 收藏的密码项目列表
     */
    QList<PasswordItem*> getFavoritePasswordItems();

    // 数据库维护操作
    /**
     * @brief 清空所有密码数据
     * @return 清空是否成功
     */
    bool clearAllPasswords();

    /**
     * @brief 获取数据库统计信息
     * @return 包含统计信息的QVariantMap
     */
    QVariantMap getDatabaseStats();

    /**
     * @brief 备份数据库到指定路径
     * @param backupPath 备份文件路径
     * @return 备份是否成功
     */
    bool backupDatabase(const QString &backupPath);

    /**
     * @brief 从备份文件恢复数据库
     * @param backupPath 备份文件路径
     * @return 恢复是否成功
     */
    bool restoreDatabase(const QString &backupPath);

    /**
     * @brief 压缩数据库（VACUUM操作）
     * @return 压缩是否成功
     */
    bool compactDatabase();

    /**
     * @brief 检查数据库完整性
     * @return 数据库是否完整
     */
    bool checkIntegrity();

    // 事务操作
    /**
     * @brief 开始数据库事务
     * @return 事务开始是否成功
     */
    bool beginTransaction();

    /**
     * @brief 提交数据库事务
     * @return 事务提交是否成功
     */
    bool commitTransaction();

    /**
     * @brief 回滚数据库事务
     * @return 事务回滚是否成功
     */
    bool rollbackTransaction();

signals:
    /**
     * @brief 数据库错误信号
     * @param error 错误信息
     */
    void databaseError(const QString &error);

    /**
     * @brief 密码项目已保存信号
     * @param item 保存的密码项目
     */
    void passwordItemSaved(PasswordItem *item);

    /**
     * @brief 密码项目已更新信号
     * @param item 更新的密码项目
     */
    void passwordItemUpdated(PasswordItem *item);

    /**
     * @brief 密码项目已删除信号
     * @param id 删除的密码项目ID
     */
    void passwordItemDeleted(int id);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager() override;

    static DatabaseManager *s_instance;  // 单例实例

    QSqlDatabase m_database;             // 数据库连接
    QString m_databasePath;              // 数据库文件路径
    bool m_isEncrypted;                  // 数据库是否已加密
    QString m_currentPassword;           // 当前数据库密码

    /**
     * @brief 创建数据库表结构
     * @return 创建是否成功
     */
    bool createTables();

    /**
     * @brief 升级数据库结构（用于版本迁移）
     * @return 升级是否成功
     */
    bool upgradeDatabase();

    /**
     * @brief 检查表是否存在
     * @param tableName 表名
     * @return 表是否存在
     */
    bool tableExists(const QString &tableName);

    /**
     * @brief 获取数据库版本
     * @return 数据库版本号
     */
    int getDatabaseVersion();

    /**
     * @brief 设置数据库版本
     * @param version 版本号
     * @return 设置是否成功
     */
    bool setDatabaseVersion(int version);

    /**
     * @brief 从查询结果创建密码项目对象
     * @param query 包含数据的查询对象
     * @return 密码项目指针
     */
    PasswordItem* createPasswordItemFromQuery(const QSqlQuery &query);

    /**
     * @brief 记录数据库错误并发出信号
     * @param operation 操作名称
     * @param error SQL错误对象
     */
    void logDatabaseError(const QString &operation, const QSqlError &error);

    /**
     * @brief 获取默认数据库文件路径
     * @return 默认数据库文件路径
     */
    QString getDefaultDatabasePath() const;
};

#endif // DATABASEMANAGER_H 