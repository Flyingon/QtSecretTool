#ifndef SQLCIPHERWRAPPER_H
#define SQLCIPHERWRAPPER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariantMap>
#include <QSqlError>

// 前向声明
struct sqlite3;

/**
 * @brief SQLCipher数据库封装类
 * 
 * 提供SQLCipher数据库的C API封装，支持数据库级别的加密
 */
class SQLCipherWrapper : public QObject
{
    Q_OBJECT

public:
    explicit SQLCipherWrapper(QObject *parent = nullptr);
    ~SQLCipherWrapper();

    /**
     * @brief 打开数据库连接
     * @param dbPath 数据库文件路径
     * @return 是否成功
     */
    bool openDatabase(const QString &dbPath);

    /**
     * @brief 关闭数据库连接
     */
    void closeDatabase();

    /**
     * @brief 设置数据库密码
     * @param password 密码
     * @return 是否成功
     */
    bool setPassword(const QString &password);

    /**
     * @brief 验证数据库密码
     * @param password 密码
     * @return 是否成功
     */
    bool verifyPassword(const QString &password);

    /**
     * @brief 更改数据库密码
     * @param oldPassword 旧密码
     * @param newPassword 新密码
     * @return 是否成功
     */
    bool changePassword(const QString &oldPassword, const QString &newPassword);

    /**
     * @brief 执行SQL语句
     * @param sql SQL语句
     * @return 是否成功
     */
    bool execute(const QString &sql);

    /**
     * @brief 执行查询并返回结果
     * @param sql SQL查询语句
     * @return 查询结果列表
     */
    QList<QVariantMap> query(const QString &sql);

    /**
     * @brief 获取最后插入的行ID
     * @return 行ID
     */
    qint64 lastInsertId() const;

    /**
     * @brief 获取影响的行数
     * @return 行数
     */
    int affectedRows() const;

    /**
     * @brief 获取最后的错误信息
     * @return 错误信息
     */
    QString lastError() const;

    /**
     * @brief 检查数据库是否已连接
     * @return 是否已连接
     */
    bool isConnected() const;

    /**
     * @brief 检查数据库是否已加密
     * @return 是否已加密
     */
    bool isEncrypted() const;

    /**
     * @brief 开始事务
     * @return 是否成功
     */
    bool beginTransaction();

    /**
     * @brief 提交事务
     * @return 是否成功
     */
    bool commitTransaction();

    /**
     * @brief 回滚事务
     * @return 是否成功
     */
    bool rollbackTransaction();

signals:
    /**
     * @brief 数据库错误信号
     * @param error 错误信息
     */
    void databaseError(const QString &error);

private:
    sqlite3 *m_db;                    ///< SQLite数据库句柄
    QString m_dbPath;                 ///< 数据库文件路径
    QString m_lastError;              ///< 最后的错误信息
    bool m_isConnected;               ///< 是否已连接
    bool m_isEncrypted;               ///< 是否已加密
    qint64 m_lastInsertId;            ///< 最后插入的行ID
    int m_affectedRows;               ///< 影响的行数

    /**
     * @brief 设置最后的错误信息
     * @param error 错误信息
     */
    void setLastError(const QString &error);

    /**
     * @brief 检查SQLCipher版本
     * @return 是否支持SQLCipher
     */
    bool checkSQLCipherVersion();
};

#endif // SQLCIPHERWRAPPER_H 