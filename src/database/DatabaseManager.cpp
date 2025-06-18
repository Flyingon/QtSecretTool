#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QVariant>
#include "../crypto/CryptoManager.h"

// 静态成员初始化
DatabaseManager* DatabaseManager::s_instance = nullptr;

// 数据库版本常量
static const int DATABASE_VERSION = 1;

/**
 * @brief 获取数据库管理器的单例实例
 * @return 数据库管理器指针
 */
DatabaseManager* DatabaseManager::instance()
{
    if (!s_instance) {
        s_instance = new DatabaseManager();
    }
    return s_instance;
}

/**
 * @brief 私有构造函数
 * @param parent 父对象指针
 */
DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    // 在构造函数中不进行数据库初始化，等待调用initialize()
}

/**
 * @brief 析构函数
 */
DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

/**
 * @brief 初始化数据库连接
 * @param databasePath 数据库文件路径，如果为空则使用默认路径
 * @return 初始化是否成功
 */
bool DatabaseManager::initialize(const QString &databasePath)
{
    // 如果已经连接，先关闭
    if (m_database.isOpen()) {
        closeDatabase();
    }

    // 确定数据库路径
    m_databasePath = databasePath.isEmpty() ? getDefaultDatabasePath() : databasePath;
    
    // 确保数据库目录存在
    QFileInfo fileInfo(m_databasePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(dir.absolutePath())) {
            qCritical() << "Failed to create database directory:" << dir.absolutePath();
            emit databaseError("Failed to create database directory");
            return false;
        }
    }

    // 创建数据库连接
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(m_databasePath);

    // 打开数据库
    if (!m_database.open()) {
        QString error = QString("Failed to open database: %1").arg(m_database.lastError().text());
        qCritical() << error;
        emit databaseError(error);
        return false;
    }

    qInfo() << "Database opened successfully:" << m_databasePath;

    // 启用外键约束
    QSqlQuery query(m_database);
    if (!query.exec("PRAGMA foreign_keys = ON")) {
        logDatabaseError("Enable foreign keys", query.lastError());
    }

    // 创建表结构
    if (!createTables()) {
        closeDatabase();
        return false;
    }

    // 检查并升级数据库
    if (!upgradeDatabase()) {
        closeDatabase();
        return false;
    }

    return true;
}

/**
 * @brief 关闭数据库连接
 */
void DatabaseManager::closeDatabase()
{
    if (m_database.isOpen()) {
        m_database.close();
        qInfo() << "Database closed";
    }
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
}

/**
 * @brief 检查数据库是否已连接
 * @return 如果数据库已连接则返回true
 */
bool DatabaseManager::isConnected() const
{
    return m_database.isOpen();
}

/**
 * @brief 保存密码项目到数据库
 * @param item 要保存的密码项目
 * @return 保存成功返回新的ID，失败返回-1
 */
int DatabaseManager::savePasswordItem(PasswordItem *item)
{
    if (!item || !isConnected()) {
        return -1;
    }

    // 获取加密管理器实例
    CryptoManager *crypto = CryptoManager::instance();
    if (!crypto->isInitialized()) {
        qCritical() << "CryptoManager not initialized";
        emit databaseError("Encryption not initialized");
        return -1;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO passwords (title, username, password, website, notes, category, created_at, updated_at, is_favorite)
        VALUES (:title, :username, :password, :website, :notes, :category, :created_at, :updated_at, :is_favorite)
    )");

    // 加密敏感字段
    QString encryptedPassword = crypto->encryptString(item->password());
    QString encryptedUsername = crypto->encryptString(item->username());
    QString encryptedNotes = crypto->encryptString(item->notes());

    query.bindValue(":title", item->title());
    query.bindValue(":username", encryptedUsername);
    query.bindValue(":password", encryptedPassword);
    query.bindValue(":website", item->website());
    query.bindValue(":notes", encryptedNotes);
    query.bindValue(":category", item->category());
    query.bindValue(":created_at", item->createdAt());
    query.bindValue(":updated_at", item->updatedAt());
    query.bindValue(":is_favorite", item->isFavorite());

    if (!query.exec()) {
        logDatabaseError("Save password item", query.lastError());
        return -1;
    }

    // 获取新插入的ID
    int newId = query.lastInsertId().toInt();
    item->setId(newId);

    qDebug() << "Password item saved with ID:" << newId;
    emit passwordItemSaved(item);
    
    return newId;
}

/**
 * @brief 更新数据库中的密码项目
 * @param item 要更新的密码项目
 * @return 更新是否成功
 */
bool DatabaseManager::updatePasswordItem(PasswordItem *item)
{
    if (!item || !isConnected() || item->id() <= 0) {
        return false;
    }

    // 获取加密管理器实例
    CryptoManager *crypto = CryptoManager::instance();
    if (!crypto->isInitialized()) {
        qCritical() << "CryptoManager not initialized";
        emit databaseError("Encryption not initialized");
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE passwords 
        SET title = :title, username = :username, password = :password, 
            website = :website, notes = :notes, category = :category, 
            updated_at = :updated_at, is_favorite = :is_favorite
        WHERE id = :id
    )");

    // 加密敏感字段
    QString encryptedPassword = crypto->encryptString(item->password());
    QString encryptedUsername = crypto->encryptString(item->username());
    QString encryptedNotes = crypto->encryptString(item->notes());

    query.bindValue(":title", item->title());
    query.bindValue(":username", encryptedUsername);
    query.bindValue(":password", encryptedPassword);
    query.bindValue(":website", item->website());
    query.bindValue(":notes", encryptedNotes);
    query.bindValue(":category", item->category());
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    query.bindValue(":is_favorite", item->isFavorite());
    query.bindValue(":id", item->id());

    if (!query.exec()) {
        logDatabaseError("Update password item", query.lastError());
        return false;
    }

    // 更新项目的时间戳
    item->setUpdatedAt(QDateTime::currentDateTime());

    qDebug() << "Password item updated, ID:" << item->id();
    emit passwordItemUpdated(item);
    
    return true;
}

/**
 * @brief 从数据库删除密码项目
 * @param id 要删除的密码项目ID
 * @return 删除是否成功
 */
bool DatabaseManager::deletePasswordItem(int id)
{
    if (!isConnected() || id <= 0) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM passwords WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        logDatabaseError("Delete password item", query.lastError());
        return false;
    }

    int affectedRows = query.numRowsAffected();
    if (affectedRows > 0) {
        qDebug() << "Password item deleted, ID:" << id;
        emit passwordItemDeleted(id);
        return true;
    }

    return false;
}

/**
 * @brief 根据ID从数据库获取密码项目
 * @param id 密码项目ID
 * @return 密码项目指针，如果未找到则返回nullptr
 */
PasswordItem* DatabaseManager::getPasswordItem(int id)
{
    if (!isConnected() || id <= 0) {
        return nullptr;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM passwords WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        logDatabaseError("Get password item", query.lastError());
        return nullptr;
    }

    if (query.next()) {
        return createPasswordItemFromQuery(query);
    }

    return nullptr;
}

/**
 * @brief 获取所有密码项目
 * @return 所有密码项目的列表
 */
QList<PasswordItem*> DatabaseManager::getAllPasswordItems()
{
    QList<PasswordItem*> items;
    
    if (!isConnected()) {
        return items;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM passwords ORDER BY updated_at DESC");

    if (!query.exec()) {
        logDatabaseError("Get all password items", query.lastError());
        return items;
    }

    while (query.next()) {
        PasswordItem *item = createPasswordItemFromQuery(query);
        if (item) {
            items.append(item);
        }
    }

    qDebug() << "Retrieved" << items.size() << "password items from database";
    return items;
}

/**
 * @brief 搜索密码项目
 * @param searchTerm 搜索词
 * @return 匹配的密码项目列表
 */
QList<PasswordItem*> DatabaseManager::searchPasswordItems(const QString &searchTerm)
{
    QList<PasswordItem*> items;
    
    if (!isConnected() || searchTerm.isEmpty()) {
        return items;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT * FROM passwords 
        WHERE title LIKE :search OR username LIKE :search OR website LIKE :search 
              OR notes LIKE :search OR category LIKE :search
        ORDER BY updated_at DESC
    )");

    QString searchPattern = QString("%%1%").arg(searchTerm);
    query.bindValue(":search", searchPattern);

    if (!query.exec()) {
        logDatabaseError("Search password items", query.lastError());
        return items;
    }

    while (query.next()) {
        PasswordItem *item = createPasswordItemFromQuery(query);
        if (item) {
            items.append(item);
        }
    }

    qDebug() << "Found" << items.size() << "password items matching search term:" << searchTerm;
    return items;
}

/**
 * @brief 根据分类获取密码项目
 * @param category 分类名称
 * @return 指定分类的密码项目列表
 */
QList<PasswordItem*> DatabaseManager::getPasswordItemsByCategory(const QString &category)
{
    QList<PasswordItem*> items;
    
    if (!isConnected()) {
        return items;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM passwords WHERE category = :category ORDER BY title");
    query.bindValue(":category", category);

    if (!query.exec()) {
        logDatabaseError("Get password items by category", query.lastError());
        return items;
    }

    while (query.next()) {
        PasswordItem *item = createPasswordItemFromQuery(query);
        if (item) {
            items.append(item);
        }
    }

    return items;
}

/**
 * @brief 获取收藏的密码项目
 * @return 收藏的密码项目列表
 */
QList<PasswordItem*> DatabaseManager::getFavoritePasswordItems()
{
    QList<PasswordItem*> items;
    
    if (!isConnected()) {
        return items;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM passwords WHERE is_favorite = 1 ORDER BY title");

    if (!query.exec()) {
        logDatabaseError("Get favorite password items", query.lastError());
        return items;
    }

    while (query.next()) {
        PasswordItem *item = createPasswordItemFromQuery(query);
        if (item) {
            items.append(item);
        }
    }

    return items;
}

/**
 * @brief 清空所有密码数据
 * @return 清空是否成功
 */
bool DatabaseManager::clearAllPasswords()
{
    if (!isConnected()) {
        return false;
    }

    QSqlQuery query(m_database);
    if (!query.exec("DELETE FROM passwords")) {
        logDatabaseError("Clear all passwords", query.lastError());
        return false;
    }

    qInfo() << "All passwords cleared from database";
    return true;
}

/**
 * @brief 获取数据库统计信息
 * @return 包含统计信息的QVariantMap
 */
QVariantMap DatabaseManager::getDatabaseStats()
{
    QVariantMap stats;
    
    if (!isConnected()) {
        return stats;
    }

    QSqlQuery query(m_database);
    
    // 总密码数量
    if (query.exec("SELECT COUNT(*) FROM passwords")) {
        if (query.next()) {
            stats["totalPasswords"] = query.value(0).toInt();
        }
    }
    
    // 收藏数量
    if (query.exec("SELECT COUNT(*) FROM passwords WHERE is_favorite = 1")) {
        if (query.next()) {
            stats["favoritePasswords"] = query.value(0).toInt();
        }
    }
    
    // 分类数量
    if (query.exec("SELECT COUNT(DISTINCT category) FROM passwords WHERE category IS NOT NULL AND category != ''")) {
        if (query.next()) {
            stats["categoriesCount"] = query.value(0).toInt();
        }
    }
    
    // 数据库文件大小
    QFileInfo fileInfo(m_databasePath);
    stats["databaseSize"] = fileInfo.size();
    stats["databasePath"] = m_databasePath;
    
    return stats;
}

/**
 * @brief 备份数据库到指定路径
 * @param backupPath 备份文件路径
 * @return 备份是否成功
 */
bool DatabaseManager::backupDatabase(const QString &backupPath)
{
    if (!isConnected() || backupPath.isEmpty()) {
        return false;
    }

    // 确保备份目录存在
    QFileInfo fileInfo(backupPath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(dir.absolutePath())) {
            qCritical() << "Failed to create backup directory:" << dir.absolutePath();
            return false;
        }
    }

    // 复制数据库文件
    if (QFile::copy(m_databasePath, backupPath)) {
        qInfo() << "Database backed up to:" << backupPath;
        return true;
    } else {
        qCritical() << "Failed to backup database to:" << backupPath;
        return false;
    }
}

/**
 * @brief 从备份文件恢复数据库
 * @param backupPath 备份文件路径
 * @return 恢复是否成功
 */
bool DatabaseManager::restoreDatabase(const QString &backupPath)
{
    if (backupPath.isEmpty() || !QFile::exists(backupPath)) {
        qCritical() << "Backup file does not exist:" << backupPath;
        return false;
    }

    // 关闭当前数据库连接
    closeDatabase();

    // 备份当前数据库文件
    QString currentBackup = m_databasePath + ".backup";
    QFile::copy(m_databasePath, currentBackup);

    // 删除当前数据库文件
    QFile::remove(m_databasePath);

    // 复制备份文件到数据库位置
    if (QFile::copy(backupPath, m_databasePath)) {
        // 重新初始化数据库
        if (initialize(m_databasePath)) {
            QFile::remove(currentBackup); // 删除临时备份
            qInfo() << "Database restored from:" << backupPath;
            return true;
        } else {
            // 恢复失败，还原原数据库
            QFile::remove(m_databasePath);
            QFile::copy(currentBackup, m_databasePath);
            QFile::remove(currentBackup);
            initialize(m_databasePath);
            qCritical() << "Failed to restore database, reverted to original";
            return false;
        }
    } else {
        // 复制失败，还原原数据库
        QFile::copy(currentBackup, m_databasePath);
        QFile::remove(currentBackup);
        initialize(m_databasePath);
        qCritical() << "Failed to copy backup file, reverted to original";
        return false;
    }
}

/**
 * @brief 压缩数据库（VACUUM操作）
 * @return 压缩是否成功
 */
bool DatabaseManager::compactDatabase()
{
    if (!isConnected()) {
        return false;
    }

    QSqlQuery query(m_database);
    if (!query.exec("VACUUM")) {
        logDatabaseError("Compact database", query.lastError());
        return false;
    }

    qInfo() << "Database compacted successfully";
    return true;
}

/**
 * @brief 检查数据库完整性
 * @return 数据库是否完整
 */
bool DatabaseManager::checkIntegrity()
{
    if (!isConnected()) {
        return false;
    }

    QSqlQuery query(m_database);
    if (!query.exec("PRAGMA integrity_check")) {
        logDatabaseError("Check integrity", query.lastError());
        return false;
    }

    if (query.next()) {
        QString result = query.value(0).toString();
        if (result == "ok") {
            qInfo() << "Database integrity check passed";
            return true;
        } else {
            qCritical() << "Database integrity check failed:" << result;
            return false;
        }
    }

    return false;
}

/**
 * @brief 开始数据库事务
 * @return 事务开始是否成功
 */
bool DatabaseManager::beginTransaction()
{
    if (!isConnected()) {
        return false;
    }

    return m_database.transaction();
}

/**
 * @brief 提交数据库事务
 * @return 事务提交是否成功
 */
bool DatabaseManager::commitTransaction()
{
    if (!isConnected()) {
        return false;
    }

    return m_database.commit();
}

/**
 * @brief 回滚数据库事务
 * @return 事务回滚是否成功
 */
bool DatabaseManager::rollbackTransaction()
{
    if (!isConnected()) {
        return false;
    }

    return m_database.rollback();
}

// 私有方法实现

/**
 * @brief 创建数据库表结构
 * @return 创建是否成功
 */
bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);

    // 创建密码表
    QString createPasswordsTable = R"(
        CREATE TABLE IF NOT EXISTS passwords (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            username TEXT,
            password TEXT NOT NULL,
            website TEXT,
            notes TEXT,
            category TEXT,
            created_at DATETIME NOT NULL,
            updated_at DATETIME NOT NULL,
            is_favorite BOOLEAN DEFAULT 0
        )
    )";

    if (!query.exec(createPasswordsTable)) {
        logDatabaseError("Create passwords table", query.lastError());
        return false;
    }

    // 创建版本表
    QString createVersionTable = R"(
        CREATE TABLE IF NOT EXISTS database_version (
            version INTEGER PRIMARY KEY
        )
    )";

    if (!query.exec(createVersionTable)) {
        logDatabaseError("Create version table", query.lastError());
        return false;
    }

    // 创建索引以提高查询性能
    QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_passwords_title ON passwords(title)",
        "CREATE INDEX IF NOT EXISTS idx_passwords_category ON passwords(category)",
        "CREATE INDEX IF NOT EXISTS idx_passwords_updated_at ON passwords(updated_at)",
        "CREATE INDEX IF NOT EXISTS idx_passwords_is_favorite ON passwords(is_favorite)"
    };

    for (const QString &index : indexes) {
        if (!query.exec(index)) {
            logDatabaseError("Create index", query.lastError());
            // 索引创建失败不是致命错误，继续执行
        }
    }

    qInfo() << "Database tables created successfully";
    return true;
}

/**
 * @brief 升级数据库结构（用于版本迁移）
 * @return 升级是否成功
 */
bool DatabaseManager::upgradeDatabase()
{
    int currentVersion = getDatabaseVersion();
    
    if (currentVersion == -1) {
        // 新数据库，设置当前版本
        return setDatabaseVersion(DATABASE_VERSION);
    }
    
    if (currentVersion == DATABASE_VERSION) {
        // 版本匹配，无需升级
        return true;
    }
    
    if (currentVersion > DATABASE_VERSION) {
        qWarning() << "Database version" << currentVersion << "is newer than expected" << DATABASE_VERSION;
        return true; // 向下兼容
    }
    
    // 这里可以添加数据库升级逻辑
    // 目前只有版本1，所以暂时不需要升级逻辑
    
    return setDatabaseVersion(DATABASE_VERSION);
}

/**
 * @brief 检查表是否存在
 * @param tableName 表名
 * @return 表是否存在
 */
bool DatabaseManager::tableExists(const QString &tableName)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=:name");
    query.bindValue(":name", tableName);
    
    if (query.exec() && query.next()) {
        return true;
    }
    
    return false;
}

/**
 * @brief 获取数据库版本
 * @return 数据库版本号
 */
int DatabaseManager::getDatabaseVersion()
{
    if (!tableExists("database_version")) {
        return -1;
    }
    
    QSqlQuery query(m_database);
    if (query.exec("SELECT version FROM database_version LIMIT 1") && query.next()) {
        return query.value(0).toInt();
    }
    
    return -1;
}

/**
 * @brief 设置数据库版本
 * @param version 版本号
 * @return 设置是否成功
 */
bool DatabaseManager::setDatabaseVersion(int version)
{
    QSqlQuery query(m_database);
    
    // 清空版本表
    if (!query.exec("DELETE FROM database_version")) {
        logDatabaseError("Clear version table", query.lastError());
        return false;
    }
    
    // 插入新版本
    query.prepare("INSERT INTO database_version (version) VALUES (:version)");
    query.bindValue(":version", version);
    
    if (!query.exec()) {
        logDatabaseError("Set database version", query.lastError());
        return false;
    }
    
    return true;
}

/**
 * @brief 从查询结果创建密码项目对象
 * @param query 包含数据的查询对象
 * @return 密码项目指针
 */
PasswordItem* DatabaseManager::createPasswordItemFromQuery(const QSqlQuery &query)
{
    // 获取加密管理器实例
    CryptoManager *crypto = CryptoManager::instance();
    if (!crypto->isInitialized()) {
        qCritical() << "CryptoManager not initialized";
        return nullptr;
    }

    PasswordItem *item = new PasswordItem();
    
    item->setId(query.value("id").toInt());
    item->setTitle(query.value("title").toString());
    
    // 解密敏感字段
    QString encryptedUsername = query.value("username").toString();
    QString encryptedPassword = query.value("password").toString();
    QString encryptedNotes = query.value("notes").toString();
    
    QString decryptedUsername = crypto->decryptString(encryptedUsername);
    QString decryptedPassword = crypto->decryptString(encryptedPassword);
    QString decryptedNotes = crypto->decryptString(encryptedNotes);
    
    item->setUsername(decryptedUsername);
    item->setPassword(decryptedPassword);
    item->setWebsite(query.value("website").toString());
    item->setNotes(decryptedNotes);
    item->setCategory(query.value("category").toString());
    item->setCreatedAt(query.value("created_at").toDateTime());
    item->setUpdatedAt(query.value("updated_at").toDateTime());
    item->setIsFavorite(query.value("is_favorite").toBool());
    
    return item;
}

/**
 * @brief 记录数据库错误并发出信号
 * @param operation 操作名称
 * @param error SQL错误对象
 */
void DatabaseManager::logDatabaseError(const QString &operation, const QSqlError &error)
{
    QString errorMessage = QString("Database error in %1: %2").arg(operation, error.text());
    qCritical() << errorMessage;
    emit databaseError(errorMessage);
}

/**
 * @brief 获取默认数据库文件路径
 * @return 默认数据库文件路径
 */
QString DatabaseManager::getDefaultDatabasePath() const
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(dataPath);
    }
    return dir.filePath("passwords.db");
} 