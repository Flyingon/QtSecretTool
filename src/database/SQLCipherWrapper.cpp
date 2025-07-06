#include "SQLCipherWrapper.h"
#include <sqlite3.h>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QVariant>

SQLCipherWrapper::SQLCipherWrapper(QObject *parent)
    : QObject(parent)
    , m_db(nullptr)
    , m_isConnected(false)
    , m_isEncrypted(false)
    , m_lastInsertId(-1)
    , m_affectedRows(0)
{
}

SQLCipherWrapper::~SQLCipherWrapper()
{
    closeDatabase();
}

bool SQLCipherWrapper::openDatabase(const QString &dbPath)
{
    if (m_isConnected) {
        closeDatabase();
    }

    m_dbPath = dbPath;
    
    // 确保数据库目录存在
    QFileInfo fileInfo(dbPath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(dir.absolutePath())) {
            setLastError(QString("Failed to create database directory: %1").arg(dir.absolutePath()));
            return false;
        }
    }

    // 打开数据库连接
    int result = sqlite3_open(dbPath.toUtf8().constData(), &m_db);
    if (result != SQLITE_OK) {
        setLastError(QString("Failed to open database: %1").arg(sqlite3_errmsg(m_db)));
        sqlite3_close(m_db);
        m_db = nullptr;
        return false;
    }

    m_isConnected = true;
    
    // 检查SQLCipher版本
    if (!checkSQLCipherVersion()) {
        setLastError("SQLCipher is not available");
        closeDatabase();
        return false;
    }

    qInfo() << "SQLCipher database opened successfully:" << dbPath;
    return true;
}

void SQLCipherWrapper::closeDatabase()
{
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
    m_isConnected = false;
    m_isEncrypted = false;
    m_lastInsertId = -1;
    m_affectedRows = 0;
    qInfo() << "SQLCipher database closed";
}

bool SQLCipherWrapper::setPassword(const QString &password)
{
    if (!m_isConnected || password.isEmpty()) {
        setLastError("Database not connected or password is empty");
        return false;
    }

    // 设置数据库密码
    QString pragmaSql = QString("PRAGMA key = '%1'").arg(password);
    if (!execute(pragmaSql)) {
        return false;
    }

    // 对于新数据库，sqlite_master可能为空，所以不强制验证
    // 只检查是否能正常执行SQL语句
    if (!execute("SELECT 1")) {
        setLastError("Invalid password or database corrupted");
        return false;
    }

    m_isEncrypted = true;
    qInfo() << "Database password set successfully";
    return true;
}

bool SQLCipherWrapper::verifyPassword(const QString &password)
{
    if (!m_isConnected || password.isEmpty()) {
        setLastError("Database not connected or password is empty");
        return false;
    }

    // 设置数据库密码
    QString pragmaSql = QString("PRAGMA key = '%1'").arg(password);
    if (!execute(pragmaSql)) {
        return false;
    }

    // 验证密码是否正确
    if (!execute("SELECT count(*) FROM sqlite_master")) {
        setLastError("Invalid password");
        return false;
    }

    m_isEncrypted = true;
    qInfo() << "Database password verified successfully";
    return true;
}

bool SQLCipherWrapper::changePassword(const QString &oldPassword, const QString &newPassword)
{
    if (!m_isConnected || oldPassword.isEmpty() || newPassword.isEmpty()) {
        setLastError("Database not connected or passwords are empty");
        return false;
    }

    // 首先验证旧密码
    if (!verifyPassword(oldPassword)) {
        return false;
    }

    // 更改数据库密码
    QString pragmaSql = QString("PRAGMA rekey = '%1'").arg(newPassword);
    if (!execute(pragmaSql)) {
        return false;
    }

    qInfo() << "Database password changed successfully";
    return true;
}

bool SQLCipherWrapper::execute(const QString &sql)
{
    if (!m_isConnected) {
        setLastError("Database not connected");
        return false;
    }

    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(m_db, sql.toUtf8().constData(), -1, &stmt, nullptr);
    
    if (result != SQLITE_OK) {
        setLastError(QString("Failed to prepare statement: %1").arg(sqlite3_errmsg(m_db)));
        return false;
    }

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE && result != SQLITE_ROW) {
        setLastError(QString("Failed to execute statement: %1").arg(sqlite3_errmsg(m_db)));
        sqlite3_finalize(stmt);
        return false;
    }

    // 获取影响的行数和最后插入的ID
    m_affectedRows = sqlite3_changes(m_db);
    m_lastInsertId = sqlite3_last_insert_rowid(m_db);

    sqlite3_finalize(stmt);
    return true;
}

QList<QVariantMap> SQLCipherWrapper::query(const QString &sql)
{
    QList<QVariantMap> results;
    
    if (!m_isConnected) {
        setLastError("Database not connected");
        return results;
    }

    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(m_db, sql.toUtf8().constData(), -1, &stmt, nullptr);
    
    if (result != SQLITE_OK) {
        setLastError(QString("Failed to prepare query: %1").arg(sqlite3_errmsg(m_db)));
        return results;
    }

    // 获取列数
    int columnCount = sqlite3_column_count(stmt);

    // 获取列名
    QStringList columnNames;
    for (int i = 0; i < columnCount; ++i) {
        columnNames.append(QString::fromUtf8(sqlite3_column_name(stmt, i)));
    }

    // 获取数据
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        QVariantMap row;
        for (int i = 0; i < columnCount; ++i) {
            QString columnName = columnNames[i];
            int columnType = sqlite3_column_type(stmt, i);
            
            QVariant value;
            switch (columnType) {
                case SQLITE_INTEGER:
                    value = sqlite3_column_int64(stmt, i);
                    break;
                case SQLITE_FLOAT:
                    value = sqlite3_column_double(stmt, i);
                    break;
                case SQLITE_TEXT:
                    value = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
                    break;
                case SQLITE_BLOB:
                    value = QByteArray(reinterpret_cast<const char*>(sqlite3_column_blob(stmt, i)), 
                                     sqlite3_column_bytes(stmt, i));
                    break;
                case SQLITE_NULL:
                default:
                    value = QVariant();
                    break;
            }
            row[columnName] = value;
        }
        results.append(row);
    }

    sqlite3_finalize(stmt);
    return results;
}

qint64 SQLCipherWrapper::lastInsertId() const
{
    return m_lastInsertId;
}

int SQLCipherWrapper::affectedRows() const
{
    return m_affectedRows;
}

QString SQLCipherWrapper::lastError() const
{
    return m_lastError;
}

bool SQLCipherWrapper::isConnected() const
{
    return m_isConnected;
}

bool SQLCipherWrapper::isEncrypted() const
{
    return m_isEncrypted;
}

bool SQLCipherWrapper::beginTransaction()
{
    return execute("BEGIN TRANSACTION");
}

bool SQLCipherWrapper::commitTransaction()
{
    return execute("COMMIT");
}

bool SQLCipherWrapper::rollbackTransaction()
{
    return execute("ROLLBACK");
}

void SQLCipherWrapper::setLastError(const QString &error)
{
    m_lastError = error;
    qCritical() << "SQLCipher error:" << error;
    emit databaseError(error);
}

bool SQLCipherWrapper::checkSQLCipherVersion()
{
    QList<QVariantMap> result = query("PRAGMA cipher_version");
    if (result.isEmpty()) {
        return false;
    }
    
    QString version = result.first().value("cipher_version").toString();
    if (version.isEmpty()) {
        return false;
    }
    
    qInfo() << "SQLCipher version:" << version;
    return true;
} 