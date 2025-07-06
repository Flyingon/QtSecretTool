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
    , m_sqlcipher(new SQLCipherWrapper(this))
    , m_isEncrypted(false)
{
    // 在构造函数中不进行数据库初始化，等待调用initialize()
    
    // 连接SQLCipher错误信号
    connect(m_sqlcipher, &SQLCipherWrapper::databaseError, 
            this, &DatabaseManager::databaseError);
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
    // 只保存路径，不做 openDatabase，不做 createTables
    m_databasePath = databasePath.isEmpty() ? getDefaultDatabasePath() : databasePath;
    return true;
}

bool DatabaseManager::openDatabase(const QString &databasePath)
{
    if (m_sqlcipher->isConnected()) {
        m_sqlcipher->closeDatabase();
    }
    m_databasePath = databasePath;
    return m_sqlcipher->openDatabase(databasePath);
}

/**
 * @brief 关闭数据库连接
 */
void DatabaseManager::closeDatabase()
{
    if (m_sqlcipher->isConnected()) {
        m_sqlcipher->closeDatabase();
        qInfo() << "SQLCipher database closed";
    }
    if (m_database.isOpen()) {
        m_database.close();
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }
}

/**
 * @brief 检查数据库是否已连接
 * @return 如果数据库已连接则返回true
 */
bool DatabaseManager::isConnected() const
{
    return m_sqlcipher->isConnected();
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

    CryptoManager *crypto = CryptoManager::instance();
    if (!crypto->isInitialized()) {
        qCritical() << "CryptoManager not initialized";
        emit databaseError("Encryption not initialized");
        return -1;
    }

    // 加密敏感字段
    QString encryptedPassword = crypto->encryptString(item->password());
    QString encryptedUsername = crypto->encryptString(item->username());
    QString encryptedNotes = crypto->encryptString(item->notes());

    QString sql = QString(R"(
        INSERT INTO passwords (title, username, password, website, notes, category, created_at, updated_at, is_favorite)
        VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', %9)
    )")
        .arg(item->title().replace("'", "''"))
        .arg(encryptedUsername.replace("'", "''"))
        .arg(encryptedPassword.replace("'", "''"))
        .arg(item->website().replace("'", "''"))
        .arg(encryptedNotes.replace("'", "''"))
        .arg(item->category().replace("'", "''"))
        .arg(item->createdAt().toString(Qt::ISODate))
        .arg(item->updatedAt().toString(Qt::ISODate))
        .arg(item->isFavorite() ? 1 : 0);
    if (!m_sqlcipher->execute(sql)) {
        emit databaseError(m_sqlcipher->lastError());
        return -1;
    }
    int newId = m_sqlcipher->lastInsertId();
    item->setId(newId);
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
    CryptoManager *crypto = CryptoManager::instance();
    if (!crypto->isInitialized()) {
        qCritical() << "CryptoManager not initialized";
        emit databaseError("Encryption not initialized");
        return false;
    }
    QString encryptedPassword = crypto->encryptString(item->password());
    QString encryptedUsername = crypto->encryptString(item->username());
    QString encryptedNotes = crypto->encryptString(item->notes());
    QString sql = QString(R"(
        UPDATE passwords SET title='%1', username='%2', password='%3', website='%4', notes='%5', category='%6', updated_at='%7', is_favorite=%8 WHERE id=%9
    )")
        .arg(item->title().replace("'", "''"))
        .arg(encryptedUsername.replace("'", "''"))
        .arg(encryptedPassword.replace("'", "''"))
        .arg(item->website().replace("'", "''"))
        .arg(encryptedNotes.replace("'", "''"))
        .arg(item->category().replace("'", "''"))
        .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
        .arg(item->isFavorite() ? 1 : 0)
        .arg(item->id());
    if (!m_sqlcipher->execute(sql)) {
        emit databaseError(m_sqlcipher->lastError());
        return false;
    }
    item->setUpdatedAt(QDateTime::currentDateTime());
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
    QString sql = QString("DELETE FROM passwords WHERE id=%1").arg(id);
    if (!m_sqlcipher->execute(sql)) {
        emit databaseError(m_sqlcipher->lastError());
        return false;
    }
    emit passwordItemDeleted(id);
    return true;
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
    QString sql = QString("SELECT * FROM passwords WHERE id=%1").arg(id);
    QList<QVariantMap> results = m_sqlcipher->query(sql);
    if (!results.isEmpty()) {
        return createPasswordItemFromMap(results.first());
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
    QList<QVariantMap> results = m_sqlcipher->query("SELECT * FROM passwords ORDER BY updated_at DESC");
    for (const QVariantMap &row : results) {
        PasswordItem *item = createPasswordItemFromMap(row);
        if (item) items.append(item);
    }
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
    QString escapedSearchTerm = searchTerm;
    escapedSearchTerm.replace("'", "''");
    QString sql = QString(R"(SELECT * FROM passwords WHERE title LIKE '%%1%' OR username LIKE '%%1%' OR website LIKE '%%1%' OR notes LIKE '%%1%' OR category LIKE '%%1%' ORDER BY updated_at DESC)").arg(escapedSearchTerm);
    QList<QVariantMap> results = m_sqlcipher->query(sql);
    for (const QVariantMap &row : results) {
        PasswordItem *item = createPasswordItemFromMap(row);
        if (item) items.append(item);
    }
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
    QString escapedCategory = category;
    escapedCategory.replace("'", "''");
    QString sql = QString("SELECT * FROM passwords WHERE category='%1' ORDER BY title").arg(escapedCategory);
    QList<QVariantMap> results = m_sqlcipher->query(sql);
    for (const QVariantMap &row : results) {
        PasswordItem *item = createPasswordItemFromMap(row);
        if (item) items.append(item);
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
    QList<QVariantMap> results = m_sqlcipher->query("SELECT * FROM passwords WHERE is_favorite = 1 ORDER BY title");
    for (const QVariantMap &row : results) {
        PasswordItem *item = createPasswordItemFromMap(row);
        if (item) items.append(item);
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
    if (!m_sqlcipher->execute("DELETE FROM passwords")) {
        emit databaseError(m_sqlcipher->lastError());
        return false;
    }
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
    QList<QVariantMap> result = m_sqlcipher->query("SELECT COUNT(*) as totalPasswords FROM passwords");
    if (!result.isEmpty()) stats["totalPasswords"] = result.first().value("totalPasswords").toInt();
    result = m_sqlcipher->query("SELECT COUNT(*) as favoritePasswords FROM passwords WHERE is_favorite = 1");
    if (!result.isEmpty()) stats["favoritePasswords"] = result.first().value("favoritePasswords").toInt();
    result = m_sqlcipher->query("SELECT COUNT(DISTINCT category) as categoriesCount FROM passwords WHERE category IS NOT NULL AND category != ''");
    if (!result.isEmpty()) stats["categoriesCount"] = result.first().value("categoriesCount").toInt();
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
    return m_sqlcipher->execute("VACUUM");
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
    QList<QVariantMap> result = m_sqlcipher->query("PRAGMA integrity_check");
    if (!result.isEmpty() && result.first().values().contains("ok")) {
        return true;
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

    if (!m_sqlcipher->execute(createPasswordsTable)) {
        qCritical() << "Failed to create passwords table:" << m_sqlcipher->lastError();
        return false;
    }

    // 创建版本表
    QString createVersionTable = R"(
        CREATE TABLE IF NOT EXISTS database_version (
            version INTEGER PRIMARY KEY
        )
    )";

    if (!m_sqlcipher->execute(createVersionTable)) {
        qCritical() << "Failed to create version table:" << m_sqlcipher->lastError();
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
        if (!m_sqlcipher->execute(index)) {
            qWarning() << "Failed to create index:" << m_sqlcipher->lastError();
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
    QString sql = QString("SELECT name FROM sqlite_master WHERE type='table' AND name='%1'").arg(tableName);
    QList<QVariantMap> result = m_sqlcipher->query(sql);
    return !result.isEmpty();
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
    
    QList<QVariantMap> result = m_sqlcipher->query("SELECT version FROM database_version LIMIT 1");
    if (!result.isEmpty()) {
        return result.first().value("version").toInt();
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
    // 清空版本表
    if (!m_sqlcipher->execute("DELETE FROM database_version")) {
        qCritical() << "Failed to clear version table:" << m_sqlcipher->lastError();
        return false;
    }
    
    // 插入新版本
    QString insertSql = QString("INSERT INTO database_version (version) VALUES (%1)").arg(version);
    if (!m_sqlcipher->execute(insertSql)) {
        qCritical() << "Failed to set database version:" << m_sqlcipher->lastError();
        return false;
    }
    
    return true;
}

/**
 * @brief 从QVariantMap组装PasswordItem
 * @param row 包含数据的QVariantMap
 * @return 密码项目指针
 */
PasswordItem* DatabaseManager::createPasswordItemFromMap(const QVariantMap &row)
{
    CryptoManager *crypto = CryptoManager::instance();
    if (!crypto->isInitialized()) {
        qCritical() << "CryptoManager not initialized";
        return nullptr;
    }
    PasswordItem *item = new PasswordItem();
    item->setId(row.value("id").toInt());
    item->setTitle(row.value("title").toString());
    QString decryptedUsername = crypto->decryptString(row.value("username").toString());
    QString decryptedPassword = crypto->decryptString(row.value("password").toString());
    QString decryptedNotes = crypto->decryptString(row.value("notes").toString());
    item->setUsername(decryptedUsername);
    item->setPassword(decryptedPassword);
    item->setWebsite(row.value("website").toString());
    item->setNotes(decryptedNotes);
    item->setCategory(row.value("category").toString());
    item->setCreatedAt(row.value("created_at").toDateTime());
    item->setUpdatedAt(row.value("updated_at").toDateTime());
    item->setIsFavorite(row.value("is_favorite").toBool());
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

/**
 * @brief 设置数据库密码（SQLCipher）
 * @param password 主密码
 * @return 设置是否成功
 */
bool DatabaseManager::setDatabasePassword(const QString &password)
{
    if (password.isEmpty()) {
        return false;
    }

    // 如果数据库未连接，先连接数据库
    if (!isConnected()) {
        if (!m_sqlcipher->openDatabase(m_databasePath)) {
            qCritical() << "Failed to open SQLCipher database:" << m_sqlcipher->lastError();
            return false;
        }
    }

    // 检查数据库是否为空（新数据库）
    // 对于新数据库，sqlite_master查询可能会失败，所以我们先尝试设置密码
    bool isEmpty = false;
    QList<QVariantMap> result = m_sqlcipher->query("SELECT count(*) FROM sqlite_master");
    if (result.isEmpty()) {
        // 查询失败，可能是新数据库
        isEmpty = true;
    } else {
        isEmpty = result.first().value("count(*)").toInt() == 0;
    }
    
    if (isEmpty) {
        // 新数据库：设置密码并创建表
        if (!m_sqlcipher->setPassword(password)) {
            qCritical() << "Failed to set database password for new database:" << m_sqlcipher->lastError();
            return false;
        }
        
        // 创建表结构
        if (!createTables()) {
            qCritical() << "Failed to create tables after encryption";
            return false;
        }
        
        // 设置数据库版本
        if (!setDatabaseVersion(DATABASE_VERSION)) {
            qCritical() << "Failed to set database version";
            return false;
        }
    } else {
        // 现有数据库：验证密码
        if (!m_sqlcipher->verifyPassword(password)) {
            qCritical() << "Failed to verify database password:" << m_sqlcipher->lastError();
            return false;
        }
    }

    m_currentPassword = password;
    m_isEncrypted = true;
    
    qInfo() << "Database password set successfully";
    return true;
}

/**
 * @brief 验证数据库密码（SQLCipher）
 * @param password 主密码
 * @return 验证是否成功
 */
bool DatabaseManager::verifyDatabasePassword(const QString &password)
{
    if (!isConnected() || password.isEmpty()) {
        return false;
    }

    if (!m_sqlcipher->verifyPassword(password)) {
        qCritical() << "Failed to verify database password:" << m_sqlcipher->lastError();
        return false;
    }

    m_currentPassword = password;
    m_isEncrypted = true;
    
    qInfo() << "Database password verified successfully";
    return true;
}

/**
 * @brief 更改数据库密码（SQLCipher）
 * @param oldPassword 旧密码
 * @param newPassword 新密码
 * @return 更改是否成功
 */
bool DatabaseManager::changeDatabasePassword(const QString &oldPassword, const QString &newPassword)
{
    if (!isConnected() || oldPassword.isEmpty() || newPassword.isEmpty()) {
        return false;
    }

    if (!m_sqlcipher->changePassword(oldPassword, newPassword)) {
        qCritical() << "Failed to change database password:" << m_sqlcipher->lastError();
        return false;
    }

    m_currentPassword = newPassword;
    
    qInfo() << "Database password changed successfully";
    return true;
}

/**
 * @brief 检查数据库是否已加密
 * @return 如果数据库已加密则返回true
 */
bool DatabaseManager::isDatabaseEncrypted() const
{
    return m_sqlcipher->isEncrypted();
}

/**
 * @brief 获取数据库文件路径
 * @return 数据库文件路径
 */
QString DatabaseManager::getDatabasePath() const
{
    return m_databasePath;
} 