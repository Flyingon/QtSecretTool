#include "PasswordManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
PasswordManager::PasswordManager(QObject *parent)
    : QObject(parent)
    , m_databaseManager(nullptr)
    , m_passwordListModel(nullptr)
    , m_isInitialized(false)
    , m_isLoading(false)
{
    // 创建密码列表模型
    m_passwordListModel = new PasswordListModel(this);
    
    qDebug() << "PasswordManager created";
}

/**
 * @brief 析构函数
 */
PasswordManager::~PasswordManager()
{
    qDebug() << "PasswordManager destroyed";
}

/**
 * @brief 获取密码总数
 * @return 密码总数
 */
int PasswordManager::totalPasswordsCount() const
{
    return m_passwordListModel ? m_passwordListModel->count() : 0;
}

/**
 * @brief 初始化密码管理器
 * @param databasePath 数据库文件路径，为空则使用默认路径
 * @return 初始化是否成功
 */
bool PasswordManager::initialize(const QString &databasePath)
{
    if (m_isInitialized) {
        qWarning() << "PasswordManager already initialized";
        return true;
    }

    setLoading(true);
    setLastError("");

    // 获取数据库管理器实例
    m_databaseManager = DatabaseManager::instance();
    
    // 连接数据库管理器的信号
    connect(m_databaseManager, &DatabaseManager::databaseError,
            this, &PasswordManager::onDatabaseError);
    connect(m_databaseManager, &DatabaseManager::passwordItemSaved,
            this, &PasswordManager::onPasswordItemSaved);
    connect(m_databaseManager, &DatabaseManager::passwordItemUpdated,
            this, &PasswordManager::onPasswordItemUpdated);
    connect(m_databaseManager, &DatabaseManager::passwordItemDeleted,
            this, &PasswordManager::onPasswordItemDeleted);

    // 初始化数据库
    if (!m_databaseManager->initialize(databasePath)) {
        setLastError("Failed to initialize database");
        setLoading(false);
        return false;
    }

    // 从数据库加载密码数据
    loadPasswordsFromDatabase();

    m_isInitialized = true;
    emit initializedChanged();
    setLoading(false);

    qInfo() << "PasswordManager initialized successfully";
    return true;
}

/**
 * @brief 创建新的密码项目
 * @param title 标题
 * @param username 用户名
 * @param password 密码
 * @param website 网站
 * @param notes 备注
 * @param category 分类
 * @return 新创建的密码项目
 */
PasswordItem* PasswordManager::createPasswordItem(const QString &title,
                                                 const QString &username,
                                                 const QString &password,
                                                 const QString &website,
                                                 const QString &notes,
                                                 const QString &category)
{
    if (!m_isInitialized) {
        setLastError("PasswordManager not initialized");
        return nullptr;
    }

    PasswordItem *item = new PasswordItem(title, username, password, website, notes, category, this);
    
    if (!validatePasswordItem(item)) {
        delete item;
        return nullptr;
    }

    return item;
}

/**
 * @brief 保存密码项目
 * @param item 要保存的密码项目
 * @return 保存是否成功
 */
bool PasswordManager::savePassword(PasswordItem *item)
{
    if (!m_isInitialized || !item) {
        setLastError("Invalid parameters for saving password");
        return false;
    }

    if (!validatePasswordItem(item)) {
        return false;
    }

    setLoading(true);

    // 保存到数据库
    int newId = m_databaseManager->savePasswordItem(item);
    if (newId <= 0) {
        setLastError("Failed to save password to database");
        setLoading(false);
        return false;
    }

    // 添加到列表模型
    m_passwordListModel->addPassword(item);
    
    setLoading(false);
    emit passwordAdded(item);
    emit totalPasswordsCountChanged();
    emit operationCompleted("save", true, "Password saved successfully");

    qDebug() << "Password saved with ID:" << newId;
    return true;
}

/**
 * @brief 更新密码项目
 * @param item 要更新的密码项目
 * @return 更新是否成功
 */
bool PasswordManager::updatePassword(PasswordItem *item)
{
    if (!m_isInitialized || !item || item->id() <= 0) {
        setLastError("Invalid parameters for updating password");
        return false;
    }

    if (!validatePasswordItem(item)) {
        return false;
    }

    setLoading(true);

    // 更新数据库
    if (!m_databaseManager->updatePasswordItem(item)) {
        setLastError("Failed to update password in database");
        setLoading(false);
        return false;
    }

    setLoading(false);
    emit passwordUpdated(item);
    emit operationCompleted("update", true, "Password updated successfully");

    qDebug() << "Password updated, ID:" << item->id();
    return true;
}

/**
 * @brief 删除密码项目
 * @param id 要删除的密码项目ID
 * @return 删除是否成功
 */
bool PasswordManager::deletePassword(int id)
{
    if (!m_isInitialized || id <= 0) {
        setLastError("Invalid ID for deleting password");
        return false;
    }

    setLoading(true);

    // 从数据库删除
    if (!m_databaseManager->deletePasswordItem(id)) {
        setLastError("Failed to delete password from database");
        setLoading(false);
        return false;
    }

    // 从列表模型中移除
    m_passwordListModel->removePasswordById(id);

    setLoading(false);
    emit passwordDeleted(id);
    emit totalPasswordsCountChanged();
    emit operationCompleted("delete", true, "Password deleted successfully");

    qDebug() << "Password deleted, ID:" << id;
    return true;
}

/**
 * @brief 根据ID获取密码项目
 * @param id 密码项目ID
 * @return 密码项目指针
 */
PasswordItem* PasswordManager::getPassword(int id)
{
    if (!m_isInitialized || id <= 0) {
        return nullptr;
    }

    return m_passwordListModel->getPasswordById(id);
}

/**
 * @brief 生成随机密码
 * @param length 密码长度
 * @param includeSymbols 是否包含特殊字符
 * @return 生成的随机密码
 */
QString PasswordManager::generatePassword(int length, bool includeSymbols)
{
    return PasswordItem::generateRandomPassword(length, includeSymbols);
}

/**
 * @brief 搜索密码
 * @param searchTerm 搜索词
 */
void PasswordManager::searchPasswords(const QString &searchTerm)
{
    if (!m_isInitialized) {
        return;
    }

    m_passwordListModel->setSearchFilter(searchTerm);
    qDebug() << "Search filter applied:" << searchTerm;
}

/**
 * @brief 获取所有分类
 * @return 分类列表
 */
QStringList PasswordManager::getCategories()
{
    if (!m_isInitialized) {
        return QStringList();
    }

    return m_passwordListModel->getCategories();
}

/**
 * @brief 按分类过滤
 * @param category 分类名称
 */
void PasswordManager::filterByCategory(const QString &category)
{
    if (!m_isInitialized) {
        return;
    }

    m_passwordListModel->setCategoryFilter(category);
    qDebug() << "Category filter applied:" << category;
}

/**
 * @brief 显示收藏的密码
 * @param showFavoritesOnly 是否只显示收藏
 */
void PasswordManager::showFavoritesOnly(bool showFavoritesOnly)
{
    if (!m_isInitialized) {
        return;
    }

    m_passwordListModel->setShowFavoritesOnly(showFavoritesOnly);
    qDebug() << "Favorites filter applied:" << showFavoritesOnly;
}

/**
 * @brief 清除所有过滤器
 */
void PasswordManager::clearFilters()
{
    if (!m_isInitialized) {
        return;
    }

    m_passwordListModel->setSearchFilter("");
    m_passwordListModel->setCategoryFilter("");
    m_passwordListModel->setShowFavoritesOnly(false);
    qDebug() << "All filters cleared";
}

/**
 * @brief 刷新密码列表
 */
void PasswordManager::refreshPasswordList()
{
    if (!m_isInitialized) {
        return;
    }

    setLoading(true);
    loadPasswordsFromDatabase();
    setLoading(false);
    qDebug() << "Password list refreshed";
}

/**
 * @brief 清空所有密码
 * @return 清空是否成功
 */
bool PasswordManager::clearAllPasswords()
{
    if (!m_isInitialized) {
        setLastError("PasswordManager not initialized");
        return false;
    }

    setLoading(true);

    // 从数据库清空
    if (!m_databaseManager->clearAllPasswords()) {
        setLastError("Failed to clear passwords from database");
        setLoading(false);
        return false;
    }

    // 清空列表模型
    m_passwordListModel->clear();

    setLoading(false);
    emit totalPasswordsCountChanged();
    emit operationCompleted("clear", true, "All passwords cleared successfully");

    qInfo() << "All passwords cleared";
    return true;
}

/**
 * @brief 获取数据库统计信息
 * @return 统计信息Map
 */
QVariantMap PasswordManager::getDatabaseStats()
{
    if (!m_isInitialized) {
        return QVariantMap();
    }

    return m_databaseManager->getDatabaseStats();
}

/**
 * @brief 备份数据库
 * @param backupPath 备份文件路径
 * @return 备份是否成功
 */
bool PasswordManager::backupDatabase(const QString &backupPath)
{
    if (!m_isInitialized || backupPath.isEmpty()) {
        setLastError("Invalid parameters for database backup");
        return false;
    }

    setLoading(true);

    bool success = m_databaseManager->backupDatabase(backupPath);
    if (success) {
        emit operationCompleted("backup", true, "Database backed up successfully");
    } else {
        setLastError("Failed to backup database");
        emit operationCompleted("backup", false, "Failed to backup database");
    }

    setLoading(false);
    return success;
}

/**
 * @brief 恢复数据库
 * @param backupPath 备份文件路径
 * @return 恢复是否成功
 */
bool PasswordManager::restoreDatabase(const QString &backupPath)
{
    if (!m_isInitialized || backupPath.isEmpty()) {
        setLastError("Invalid parameters for database restore");
        return false;
    }

    setLoading(true);

    bool success = m_databaseManager->restoreDatabase(backupPath);
    if (success) {
        // 重新加载密码数据
        loadPasswordsFromDatabase();
        emit operationCompleted("restore", true, "Database restored successfully");
    } else {
        setLastError("Failed to restore database");
        emit operationCompleted("restore", false, "Failed to restore database");
    }

    setLoading(false);
    return success;
}

/**
 * @brief 导出密码数据为JSON格式
 * @param filePath 导出文件路径
 * @param includePasswords 是否包含密码明文
 * @return 导出是否成功
 */
bool PasswordManager::exportToJson(const QString &filePath, bool includePasswords)
{
    if (!m_isInitialized || filePath.isEmpty()) {
        setLastError("Invalid parameters for JSON export");
        return false;
    }

    setLoading(true);

    QJsonObject rootObject;
    QJsonArray passwordsArray;

    // 获取所有密码项目
    QList<PasswordItem*> allPasswords = m_passwordListModel->getAllPasswords();
    
    for (PasswordItem *item : allPasswords) {
        QJsonObject passwordObject;
        passwordObject["title"] = item->title();
        passwordObject["username"] = item->username();
        
        if (includePasswords) {
            passwordObject["password"] = item->password();
        } else {
            passwordObject["password"] = "***HIDDEN***";
        }
        
        passwordObject["website"] = item->website();
        passwordObject["notes"] = item->notes();
        passwordObject["category"] = item->category();
        passwordObject["createdAt"] = item->createdAt().toString(Qt::ISODate);
        passwordObject["updatedAt"] = item->updatedAt().toString(Qt::ISODate);
        passwordObject["isFavorite"] = item->isFavorite();
        
        passwordsArray.append(passwordObject);
    }

    rootObject["passwords"] = passwordsArray;
    rootObject["exportDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    rootObject["includePasswords"] = includePasswords;
    rootObject["totalCount"] = allPasswords.size();

    // 写入文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        setLastError("Failed to open file for writing: " + filePath);
        setLoading(false);
        return false;
    }

    QJsonDocument document(rootObject);
    file.write(document.toJson());
    file.close();

    setLoading(false);
    emit operationCompleted("export_json", true, QString("Exported %1 passwords to JSON").arg(allPasswords.size()));
    
    qInfo() << "Exported" << allPasswords.size() << "passwords to JSON:" << filePath;
    return true;
}

/**
 * @brief 从JSON文件导入密码数据
 * @param filePath 导入文件路径
 * @param mergeMode 是否为合并模式（true为合并，false为替换）
 * @return 导入是否成功
 */
bool PasswordManager::importFromJson(const QString &filePath, bool mergeMode)
{
    if (!m_isInitialized || filePath.isEmpty()) {
        setLastError("Invalid parameters for JSON import");
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        setLastError("Failed to open file for reading: " + filePath);
        return false;
    }

    setLoading(true);

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        setLastError("JSON parse error: " + error.errorString());
        setLoading(false);
        return false;
    }

    QJsonObject rootObject = document.object();
    QJsonArray passwordsArray = rootObject["passwords"].toArray();

    if (passwordsArray.isEmpty()) {
        setLastError("No passwords found in JSON file");
        setLoading(false);
        return false;
    }

    // 如果不是合并模式，清空现有密码
    if (!mergeMode) {
        if (!clearAllPasswords()) {
            setLastError("Failed to clear existing passwords");
            setLoading(false);
            return false;
        }
    }

    int importedCount = 0;
    int totalCount = passwordsArray.size();

    // 开始数据库事务
    m_databaseManager->beginTransaction();

    for (int i = 0; i < passwordsArray.size(); ++i) {
        QJsonObject passwordObject = passwordsArray[i].toObject();
        
        QString title = passwordObject["title"].toString();
        QString username = passwordObject["username"].toString();
        QString password = passwordObject["password"].toString();
        QString website = passwordObject["website"].toString();
        QString notes = passwordObject["notes"].toString();
        QString category = passwordObject["category"].toString();
        bool isFavorite = passwordObject["isFavorite"].toBool();

        // 跳过隐藏的密码
        if (password == "***HIDDEN***") {
            continue;
        }

        // 创建新的密码项目
        PasswordItem *item = new PasswordItem(title, username, password, website, notes, category, this);
        item->setIsFavorite(isFavorite);

        // 设置时间戳（如果文件中有的话）
        QString createdAtStr = passwordObject["createdAt"].toString();
        QString updatedAtStr = passwordObject["updatedAt"].toString();
        
        if (!createdAtStr.isEmpty()) {
            item->setCreatedAt(QDateTime::fromString(createdAtStr, Qt::ISODate));
        }
        if (!updatedAtStr.isEmpty()) {
            item->setUpdatedAt(QDateTime::fromString(updatedAtStr, Qt::ISODate));
        }

        // 保存到数据库
        int newId = m_databaseManager->savePasswordItem(item);
        if (newId > 0) {
            m_passwordListModel->addPassword(item);
            importedCount++;
        } else {
            delete item;
        }

        // 更新进度
        emit importProgressUpdated(i + 1, totalCount);
    }

    // 提交事务
    if (importedCount > 0) {
        m_databaseManager->commitTransaction();
    } else {
        m_databaseManager->rollbackTransaction();
    }

    setLoading(false);
    emit totalPasswordsCountChanged();
    
    if (importedCount > 0) {
        emit operationCompleted("import_json", true, QString("Imported %1 passwords from JSON").arg(importedCount));
        qInfo() << "Imported" << importedCount << "passwords from JSON:" << filePath;
        return true;
    } else {
        setLastError("No passwords were imported");
        emit operationCompleted("import_json", false, "No passwords were imported");
        return false;
    }
}

/**
 * @brief 导出密码数据为CSV格式
 * @param filePath 导出文件路径
 * @param includePasswords 是否包含密码明文
 * @return 导出是否成功
 */
bool PasswordManager::exportToCsv(const QString &filePath, bool includePasswords)
{
    if (!m_isInitialized || filePath.isEmpty()) {
        setLastError("Invalid parameters for CSV export");
        return false;
    }

    setLoading(true);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        setLastError("Failed to open file for writing: " + filePath);
        setLoading(false);
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // 写入CSV头部
    stream << "Title,Username,Password,Website,Notes,Category,Created,Updated,Favorite\n";

    // 获取所有密码项目
    QList<PasswordItem*> allPasswords = m_passwordListModel->getAllPasswords();
    
    for (PasswordItem *item : allPasswords) {
        // 转义CSV字段中的逗号和引号
        auto escapeField = [](const QString &field) -> QString {
            QString escaped = field;
            escaped.replace("\"", "\"\"");
            if (escaped.contains(",") || escaped.contains("\"") || escaped.contains("\n")) {
                escaped = "\"" + escaped + "\"";
            }
            return escaped;
        };

        stream << escapeField(item->title()) << ",";
        stream << escapeField(item->username()) << ",";
        
        if (includePasswords) {
            stream << escapeField(item->password()) << ",";
        } else {
            stream << "***HIDDEN***,";
        }
        
        stream << escapeField(item->website()) << ",";
        stream << escapeField(item->notes()) << ",";
        stream << escapeField(item->category()) << ",";
        stream << item->createdAt().toString(Qt::ISODate) << ",";
        stream << item->updatedAt().toString(Qt::ISODate) << ",";
        stream << (item->isFavorite() ? "Yes" : "No") << "\n";
    }

    file.close();
    setLoading(false);
    emit operationCompleted("export_csv", true, QString("Exported %1 passwords to CSV").arg(allPasswords.size()));
    
    qInfo() << "Exported" << allPasswords.size() << "passwords to CSV:" << filePath;
    return true;
}

/**
 * @brief 从CSV文件导入密码数据
 * @param filePath 导入文件路径
 * @param mergeMode 是否为合并模式
 * @return 导入是否成功
 */
bool PasswordManager::importFromCsv(const QString &filePath, bool mergeMode)
{
    if (!m_isInitialized || filePath.isEmpty()) {
        setLastError("Invalid parameters for CSV import");
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setLastError("Failed to open file for reading: " + filePath);
        return false;
    }

    setLoading(true);

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // 跳过头部行
    if (!stream.atEnd()) {
        stream.readLine();
    }

    // 如果不是合并模式，清空现有密码
    if (!mergeMode) {
        if (!clearAllPasswords()) {
            setLastError("Failed to clear existing passwords");
            setLoading(false);
            file.close();
            return false;
        }
    }

    int importedCount = 0;
    int lineNumber = 1;

    // 开始数据库事务
    m_databaseManager->beginTransaction();

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        lineNumber++;

        if (line.trimmed().isEmpty()) {
            continue;
        }

        // 简单的CSV解析（支持带引号的字段）
        QStringList fields;
        QString currentField;
        bool inQuotes = false;
        
        for (int i = 0; i < line.length(); ++i) {
            QChar c = line[i];
            
            if (c == '"') {
                if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                    // 转义的引号
                    currentField += '"';
                    i++; // 跳过下一个引号
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                fields.append(currentField);
                currentField.clear();
            } else {
                currentField += c;
            }
        }
        fields.append(currentField);

        if (fields.size() < 6) {
            qWarning() << "Invalid CSV line at" << lineNumber << ":" << line;
            continue;
        }

        QString title = fields[0];
        QString username = fields[1];
        QString password = fields[2];
        QString website = fields[3];
        QString notes = fields[4];
        QString category = fields[5];

        // 跳过隐藏的密码
        if (password == "***HIDDEN***" || title.isEmpty()) {
            continue;
        }

        // 创建新的密码项目
        PasswordItem *item = new PasswordItem(title, username, password, website, notes, category, this);

        // 设置时间戳和收藏状态（如果有的话）
        if (fields.size() > 6 && !fields[6].isEmpty()) {
            item->setCreatedAt(QDateTime::fromString(fields[6], Qt::ISODate));
        }
        if (fields.size() > 7 && !fields[7].isEmpty()) {
            item->setUpdatedAt(QDateTime::fromString(fields[7], Qt::ISODate));
        }
        if (fields.size() > 8) {
            item->setIsFavorite(fields[8].toLower() == "yes" || fields[8].toLower() == "true");
        }

        // 保存到数据库
        int newId = m_databaseManager->savePasswordItem(item);
        if (newId > 0) {
            m_passwordListModel->addPassword(item);
            importedCount++;
        } else {
            delete item;
        }
    }

    file.close();

    // 提交事务
    if (importedCount > 0) {
        m_databaseManager->commitTransaction();
    } else {
        m_databaseManager->rollbackTransaction();
    }

    setLoading(false);
    emit totalPasswordsCountChanged();
    
    if (importedCount > 0) {
        emit operationCompleted("import_csv", true, QString("Imported %1 passwords from CSV").arg(importedCount));
        qInfo() << "Imported" << importedCount << "passwords from CSV:" << filePath;
        return true;
    } else {
        setLastError("No passwords were imported");
        emit operationCompleted("import_csv", false, "No passwords were imported");
        return false;
    }
}

// 私有槽函数实现

/**
 * @brief 数据库错误处理槽
 * @param error 错误信息
 */
void PasswordManager::onDatabaseError(const QString &error)
{
    setLastError(error);
    qCritical() << "Database error:" << error;
}

/**
 * @brief 密码项目保存完成槽
 * @param item 保存的密码项目
 */
void PasswordManager::onPasswordItemSaved(PasswordItem *item)
{
    Q_UNUSED(item)
    // 数据库层已处理，这里可以添加额外的处理逻辑
}

/**
 * @brief 密码项目更新完成槽
 * @param item 更新的密码项目
 */
void PasswordManager::onPasswordItemUpdated(PasswordItem *item)
{
    Q_UNUSED(item)
    // 数据库层已处理，这里可以添加额外的处理逻辑
}

/**
 * @brief 密码项目删除完成槽
 * @param id 删除的密码项目ID
 */
void PasswordManager::onPasswordItemDeleted(int id)
{
    Q_UNUSED(id)
    // 数据库层已处理，这里可以添加额外的处理逻辑
}

// 私有方法实现

/**
 * @brief 设置错误信息
 * @param error 错误信息
 */
void PasswordManager::setLastError(const QString &error)
{
    if (m_lastError != error) {
        m_lastError = error;
        emit lastErrorChanged();
    }
}

/**
 * @brief 设置加载状态
 * @param loading 是否正在加载
 */
void PasswordManager::setLoading(bool loading)
{
    if (m_isLoading != loading) {
        m_isLoading = loading;
        emit loadingChanged();
    }
}

/**
 * @brief 从数据库加载所有密码
 */
void PasswordManager::loadPasswordsFromDatabase()
{
    if (!m_databaseManager || !m_databaseManager->isConnected()) {
        return;
    }

    QList<PasswordItem*> passwords = m_databaseManager->getAllPasswordItems();
    m_passwordListModel->setPasswordItems(passwords);
    emit totalPasswordsCountChanged();

    qDebug() << "Loaded" << passwords.size() << "passwords from database";
}

/**
 * @brief 验证密码项目数据
 * @param item 要验证的密码项目
 * @return 验证结果
 */
bool PasswordManager::validatePasswordItem(PasswordItem *item) const
{
    if (!item) {
        return false;
    }

    if (item->title().trimmed().isEmpty()) {
        const_cast<PasswordManager*>(this)->setLastError("Password title cannot be empty");
        return false;
    }

    if (item->password().isEmpty()) {
        const_cast<PasswordManager*>(this)->setLastError("Password cannot be empty");
        return false;
    }

    return true;
} 