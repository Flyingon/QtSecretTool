#include "PasswordManager.h"
#include <QDebug>
#include <QSettings>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStandardPaths>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
PasswordManager::PasswordManager(QObject *parent)
    : QObject(parent)
    , m_cryptoManager(CryptoManager::instance())
    , m_databaseManager(DatabaseManager::instance())
    , m_passwordListModel(new PasswordListModel(this))
    , m_isLoading(false)
{
    // 连接加密管理器的错误信号
    connect(m_cryptoManager, &CryptoManager::cryptoError, 
            this, &PasswordManager::passwordError);
    
    // 连接数据库管理器的信号
    connect(m_databaseManager, &DatabaseManager::databaseError,
            this, &PasswordManager::setLastError);
    connect(m_databaseManager, &DatabaseManager::passwordItemSaved,
            this, &PasswordManager::refreshPasswordList);
    connect(m_databaseManager, &DatabaseManager::passwordItemUpdated,
            this, &PasswordManager::refreshPasswordList);
    connect(m_databaseManager, &DatabaseManager::passwordItemDeleted,
            this, &PasswordManager::refreshPasswordList);
}

/**
 * @brief 检查是否已设置主密码
 * @return 如果已设置主密码则返回true
 */
bool PasswordManager::hasMasterPassword() const
{
    // 检查数据库文件是否存在
    QString dbPath = m_databaseManager->getDatabasePath();
    if (!QFile::exists(dbPath)) {
        // 数据库文件不存在，肯定没有设置主密码
        return false;
    }
    
    // 检查数据库文件大小，如果文件存在且大于0字节，说明已经设置过主密码
    QFileInfo fileInfo(dbPath);
    if (fileInfo.size() > 0) {
        return true;
    }
    
    // 如果文件存在但为空，删除它并返回false
    QFile::remove(dbPath);
    return false;
}

/**
 * @brief 设置主密码
 * @param password 主密码
 * @return 设置是否成功
 */
bool PasswordManager::setMasterPassword(const QString &password)
{
    if (password.isEmpty()) {
        setLastError("主密码不能为空");
        return false;
    }

    // 先打开数据库
    if (!m_databaseManager->openDatabase(m_databaseManager->getDatabasePath())) {
        setLastError("打开数据库失败");
        return false;
    }

    // 设置数据库密码（SQLCipher）
    if (!m_databaseManager->setDatabasePassword(password)) {
        setLastError("设置数据库密码失败");
        return false;
    }

    // 创建表结构（新库）
    m_databaseManager->createTables();

    // 初始化加密管理器
    if (!m_cryptoManager->initialize(password)) {
        setLastError("初始化加密管理器失败");
        return false;
    }

    // 保存设置
    QSettings settings;
    settings.setValue("master_password_set", "true");

    qInfo() << "Master password set successfully";
    emit masterPasswordSet();
    return true;
}

/**
 * @brief 验证主密码
 * @param password 要验证的主密码
 * @return 密码是否正确
 */
bool PasswordManager::verifyMasterPassword(const QString &password)
{
    if (password.isEmpty()) {
        setLastError("主密码不能为空");
        return false;
    }

    // 先打开数据库
    if (!m_databaseManager->openDatabase(m_databaseManager->getDatabasePath())) {
        setLastError("打开数据库失败");
        return false;
    }

    // 验证数据库密码（SQLCipher）
    if (!m_databaseManager->verifyDatabasePassword(password)) {
        setLastError("数据库密码验证失败");
        return false;
    }

    // 验证成功后初始化加密管理器
    if (!m_cryptoManager->initialize(password)) {
        setLastError("初始化加密管理器失败");
        return false;
    }

    qInfo() << "Master password verified successfully";
    emit masterPasswordVerified();
    return true;
}

/**
 * @brief 更改主密码
 * @param oldPassword 旧主密码
 * @param newPassword 新主密码
 * @return 更改是否成功
 */
bool PasswordManager::changeMasterPassword(const QString &oldPassword, const QString &newPassword)
{
    if (oldPassword.isEmpty() || newPassword.isEmpty()) {
        setLastError("密码不能为空");
        return false;
    }

    // 更改数据库密码（SQLCipher）
    if (!m_databaseManager->changeDatabasePassword(oldPassword, newPassword)) {
        setLastError("更改数据库密码失败");
        return false;
    }

    // 更改加密管理器密码
    if (!m_cryptoManager->changeMasterPassword(oldPassword, newPassword)) {
        setLastError("更改加密管理器密码失败");
        return false;
    }

    qInfo() << "Master password changed successfully";
    emit masterPasswordChanged();
    return true;
}

/**
 * @brief 初始化加密管理器
 * @param password 主密码
 * @return 初始化是否成功
 */
bool PasswordManager::initializeCrypto(const QString &password)
{
    if (password.isEmpty()) {
        setLastError("主密码不能为空");
        return false;
    }

    if (!m_cryptoManager->initialize(password)) {
        setLastError("初始化加密管理器失败");
        return false;
    }

    return true;
}

/**
 * @brief 检查加密管理器是否已初始化
 * @return 如果已初始化则返回true
 */
bool PasswordManager::isCryptoInitialized() const
{
    return m_cryptoManager->isInitialized();
}

/**
 * @brief 设置数据库密码（SQLCipher）
 * @param password 主密码
 * @return 设置是否成功
 */
bool PasswordManager::setDatabasePassword(const QString &password)
{
    if (password.isEmpty()) {
        setLastError("数据库密码不能为空");
        return false;
    }

    if (!m_databaseManager->setDatabasePassword(password)) {
        setLastError("设置数据库密码失败");
        return false;
    }

    qInfo() << "Database password set successfully";
    return true;
}

/**
 * @brief 验证数据库密码（SQLCipher）
 * @param password 主密码
 * @return 验证是否成功
 */
bool PasswordManager::verifyDatabasePassword(const QString &password)
{
    if (password.isEmpty()) {
        setLastError("数据库密码不能为空");
        return false;
    }

    if (!m_databaseManager->verifyDatabasePassword(password)) {
        setLastError("数据库密码验证失败");
        return false;
    }

    qInfo() << "Database password verified successfully";
    return true;
}

/**
 * @brief 更改数据库密码（SQLCipher）
 * @param oldPassword 旧密码
 * @param newPassword 新密码
 * @return 更改是否成功
 */
bool PasswordManager::changeDatabasePassword(const QString &oldPassword, const QString &newPassword)
{
    if (oldPassword.isEmpty() || newPassword.isEmpty()) {
        setLastError("密码不能为空");
        return false;
    }

    if (!m_databaseManager->changeDatabasePassword(oldPassword, newPassword)) {
        setLastError("更改数据库密码失败");
        return false;
    }

    qInfo() << "Database password changed successfully";
    return true;
}

/**
 * @brief 检查数据库是否已加密
 * @return 如果数据库已加密则返回true
 */
bool PasswordManager::isDatabaseEncrypted() const
{
    return m_databaseManager->isDatabaseEncrypted();
}

/**
 * @brief 初始化密码管理器
 * @return 初始化是否成功
 */
bool PasswordManager::initialize()
{
    qInfo() << "Initializing PasswordManager...";
    
    // 检查是否已设置主密码
    if (!hasMasterPassword()) {
        qInfo() << "No master password set, will be set on first use";
        return true;
    }
    
    // 只有在加密管理器已初始化时才刷新密码列表
    if (m_cryptoManager->isInitialized()) {
        refreshPasswordList();
    } else {
        qInfo() << "CryptoManager not initialized, password list will be loaded after master password verification";
    }
    
    qInfo() << "PasswordManager initialized successfully";
    return true;
}

/**
 * @brief 生成随机密码
 * @param length 密码长度
 * @param includeSymbols 是否包含特殊符号
 * @return 生成的密码
 */
QString PasswordManager::generatePassword(int length, bool includeSymbols)
{
    const QString lowercase = "abcdefghijklmnopqrstuvwxyz";
    const QString uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const QString digits = "0123456789";
    const QString symbols = "!@#$%^&*()_+-=[]{}|;:,.<>?";
    
    QString allChars = lowercase + uppercase + digits;
    if (includeSymbols) {
        allChars += symbols;
    }
    
    QString password;
    QRandomGenerator *generator = QRandomGenerator::global();
    
    // 确保至少包含每种字符类型
    if (length >= 4) {
        password += lowercase[generator->bounded(lowercase.length())];
        password += uppercase[generator->bounded(uppercase.length())];
        password += digits[generator->bounded(digits.length())];
        if (includeSymbols) {
            password += symbols[generator->bounded(symbols.length())];
        }
        
        // 填充剩余长度
        for (int i = password.length(); i < length; ++i) {
            password += allChars[generator->bounded(allChars.length())];
        }
        
        // 打乱密码字符顺序
        for (int i = password.length() - 1; i > 0; --i) {
            int j = generator->bounded(i + 1);
            QChar temp = password[i];
            password[i] = password[j];
            password[j] = temp;
        }
    } else {
        // 如果长度太短，直接生成
        for (int i = 0; i < length; ++i) {
            password += allChars[generator->bounded(allChars.length())];
        }
    }
    
    return password;
}

/**
 * @brief 创建密码项目
 */
PasswordItem* PasswordManager::createPasswordItem(const QString &title, 
                                                const QString &username, 
                                                const QString &password,
                                                const QString &website,
                                                const QString &notes,
                                                const QString &category)
{
    PasswordItem *item = new PasswordItem(title, username, password, website, notes, category, this);
    return item;
}

/**
 * @brief 保存密码
 */
bool PasswordManager::savePassword(PasswordItem *item)
{
    if (!item) {
        setLastError("密码项目为空");
        return false;
    }

    setLoading(true);
    clearLastError();

    int id = m_databaseManager->savePasswordItem(item);
    if (id > 0) {
        setLoading(false);
        return true;
    } else {
        setLastError("保存密码失败");
        setLoading(false);
        return false;
    }
}

/**
 * @brief 更新密码
 */
bool PasswordManager::updatePassword(PasswordItem *item)
{
    if (!item) {
        setLastError("密码项目为空");
        return false;
    }

    setLoading(true);
    clearLastError();

    bool success = m_databaseManager->updatePasswordItem(item);
    setLoading(false);
    
    if (!success) {
        setLastError("更新密码失败");
    }
    
    return success;
}

/**
 * @brief 删除密码
 */
bool PasswordManager::deletePassword(int id)
{
    setLoading(true);
    clearLastError();

    bool success = m_databaseManager->deletePasswordItem(id);
    setLoading(false);
    
    if (!success) {
        setLastError("删除密码失败");
    }
    
    return success;
}

/**
 * @brief 搜索密码
 */
void PasswordManager::searchPasswords(const QString &searchTerm)
{
    setLoading(true);
    
    QList<PasswordItem*> items = m_databaseManager->searchPasswordItems(searchTerm);
    m_passwordListModel->setPasswordItems(items);
    
    setLoading(false);
    emit totalPasswordsCountChanged();
}

/**
 * @brief 显示收藏夹
 */
void PasswordManager::showFavoritesOnly(bool showOnly)
{
    setLoading(true);
    
    QList<PasswordItem*> items;
    if (showOnly) {
        items = m_databaseManager->getFavoritePasswordItems();
    } else {
        items = m_databaseManager->getAllPasswordItems();
    }
    
    m_passwordListModel->setPasswordItems(items);
    setLoading(false);
    emit totalPasswordsCountChanged();
}

/**
 * @brief 清除过滤器
 */
void PasswordManager::clearFilters()
{
    setLoading(true);
    
    QList<PasswordItem*> items = m_databaseManager->getAllPasswordItems();
    m_passwordListModel->setPasswordItems(items);
    
    setLoading(false);
    emit totalPasswordsCountChanged();
}

/**
 * @brief 按分类过滤
 */
void PasswordManager::filterByCategory(const QString &category)
{
    setLoading(true);
    
    QList<PasswordItem*> items = m_databaseManager->getPasswordItemsByCategory(category);
    m_passwordListModel->setPasswordItems(items);
    
    setLoading(false);
    emit totalPasswordsCountChanged();
}

/**
 * @brief 获取所有分类
 */
QStringList PasswordManager::getCategories()
{
    QStringList categories;
    QList<PasswordItem*> items = m_databaseManager->getAllPasswordItems();
    
    for (PasswordItem *item : items) {
        if (!item->category().isEmpty() && !categories.contains(item->category())) {
            categories.append(item->category());
        }
    }
    
    categories.sort();
    return categories;
}

/**
 * @brief 刷新密码列表
 */
void PasswordManager::refreshPasswordList()
{
    setLoading(true);
    
    QList<PasswordItem*> items = m_databaseManager->getAllPasswordItems();
    m_passwordListModel->setPasswordItems(items);
    
    setLoading(false);
    emit totalPasswordsCountChanged();
}

/**
 * @brief 获取密码总数
 */
int PasswordManager::totalPasswordsCount() const
{
    return m_passwordListModel->rowCount();
}

/**
 * @brief 备份数据库
 */
bool PasswordManager::backupDatabase(const QString &filePath)
{
    setLoading(true);
    clearLastError();

    bool success = m_databaseManager->backupDatabase(filePath);
    setLoading(false);
    
    if (!success) {
        setLastError("备份数据库失败");
    }
    
    return success;
}

/**
 * @brief 恢复数据库
 */
bool PasswordManager::restoreDatabase(const QString &filePath)
{
    setLoading(true);
    clearLastError();

    bool success = m_databaseManager->restoreDatabase(filePath);
    setLoading(false);
    
    if (!success) {
        setLastError("恢复数据库失败");
    } else {
        refreshPasswordList();
    }
    
    return success;
}

/**
 * @brief 导出为JSON
 */
bool PasswordManager::exportToJson(const QString &filePath, bool includePasswords)
{
    setLoading(true);
    clearLastError();

    QList<PasswordItem*> items = m_databaseManager->getAllPasswordItems();
    QJsonArray jsonArray;
    
    for (PasswordItem *item : items) {
        QJsonObject jsonItem;
        jsonItem["title"] = item->title();
        jsonItem["username"] = includePasswords ? item->username() : "";
        jsonItem["password"] = includePasswords ? item->password() : "";
        jsonItem["website"] = item->website();
        jsonItem["notes"] = includePasswords ? item->notes() : "";
        jsonItem["category"] = item->category();
        jsonItem["created_at"] = item->createdAt().toString(Qt::ISODate);
        jsonItem["updated_at"] = item->updatedAt().toString(Qt::ISODate);
        jsonItem["is_favorite"] = item->isFavorite();
        
        jsonArray.append(jsonItem);
    }
    
    QJsonDocument doc(jsonArray);
    QFile file(filePath);
    
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        setLoading(false);
        return true;
    } else {
        setLastError("导出JSON失败");
        setLoading(false);
        return false;
    }
}

/**
 * @brief 导出为CSV
 */
bool PasswordManager::exportToCsv(const QString &filePath, bool includePasswords)
{
    setLoading(true);
    clearLastError();

    QList<PasswordItem*> items = m_databaseManager->getAllPasswordItems();
    QFile file(filePath);
    
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        
        // 写入标题行
        stream << "Title,Username,Password,Website,Notes,Category,Created At,Updated At,Is Favorite\n";
        
        // 写入数据行
        for (PasswordItem *item : items) {
            QStringList fields;
            fields << item->title();
            fields << (includePasswords ? item->username() : "");
            fields << (includePasswords ? item->password() : "");
            fields << item->website();
            fields << (includePasswords ? item->notes() : "");
            fields << item->category();
            fields << item->createdAt().toString(Qt::ISODate);
            fields << item->updatedAt().toString(Qt::ISODate);
            fields << (item->isFavorite() ? "true" : "false");
            
            // 转义CSV字段
            for (int i = 0; i < fields.size(); ++i) {
                QString field = fields[i];
                if (field.contains(",") || field.contains("\"") || field.contains("\n")) {
                    field = "\"" + field.replace("\"", "\"\"") + "\"";
                    fields[i] = field;
                }
            }
            
            stream << fields.join(",") << "\n";
        }
        
        file.close();
        setLoading(false);
        return true;
    } else {
        setLastError("导出CSV失败");
        setLoading(false);
        return false;
    }
}

/**
 * @brief 从JSON导入
 */
bool PasswordManager::importFromJson(const QString &filePath, bool overwrite)
{
    setLoading(true);
    clearLastError();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        setLastError("无法打开JSON文件");
        setLoading(false);
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        setLastError("JSON格式错误");
        setLoading(false);
        return false;
    }
    
    QJsonArray jsonArray = doc.array();
    int successCount = 0;
    
    for (const QJsonValue &value : jsonArray) {
        if (value.isObject()) {
            QJsonObject obj = value.toObject();
            
            PasswordItem *item = new PasswordItem(
                obj["title"].toString(),
                obj["username"].toString(),
                obj["password"].toString(),
                obj["website"].toString(),
                obj["notes"].toString(),
                obj["category"].toString()
            );
            
            item->setCreatedAt(QDateTime::fromString(obj["created_at"].toString(), Qt::ISODate));
            item->setUpdatedAt(QDateTime::fromString(obj["updated_at"].toString(), Qt::ISODate));
            item->setIsFavorite(obj["is_favorite"].toBool());
            
            if (m_databaseManager->savePasswordItem(item) > 0) {
                successCount++;
            }
        }
    }
    
    setLoading(false);
    refreshPasswordList();
    
    if (successCount == 0) {
        setLastError("导入失败，没有成功导入任何密码");
        return false;
    }
    
    return true;
}

/**
 * @brief 从CSV导入
 */
bool PasswordManager::importFromCsv(const QString &filePath, bool overwrite)
{
    setLoading(true);
    clearLastError();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setLastError("无法打开CSV文件");
        setLoading(false);
        return false;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    
    // 跳过标题行
    QString line = stream.readLine();
    if (line.isNull()) {
        setLastError("CSV文件为空");
        setLoading(false);
        return false;
    }
    
    int successCount = 0;
    while (!stream.atEnd()) {
        line = stream.readLine();
        if (line.isNull()) break;
        
        QStringList fields = parseCsvLine(line);
        if (fields.size() >= 9) {
            PasswordItem *item = new PasswordItem(
                fields[0], // title
                fields[1], // username
                fields[2], // password
                fields[3], // website
                fields[4], // notes
                fields[5]  // category
            );
            
            item->setCreatedAt(QDateTime::fromString(fields[6], Qt::ISODate));
            item->setUpdatedAt(QDateTime::fromString(fields[7], Qt::ISODate));
            item->setIsFavorite(fields[8] == "true");
            
            if (m_databaseManager->savePasswordItem(item) > 0) {
                successCount++;
            }
        }
    }
    
    file.close();
    setLoading(false);
    refreshPasswordList();
    
    if (successCount == 0) {
        setLastError("导入失败，没有成功导入任何密码");
        return false;
    }
    
    return true;
}

/**
 * @brief 清除所有密码
 */
bool PasswordManager::clearAllPasswords()
{
    setLoading(true);
    clearLastError();

    bool success = m_databaseManager->clearAllPasswords();
    setLoading(false);
    
    if (success) {
        refreshPasswordList();
    } else {
        setLastError("清除所有密码失败");
    }
    
    return success;
}

/**
 * @brief 设置加载状态
 */
void PasswordManager::setLoading(bool loading)
{
    if (m_isLoading != loading) {
        m_isLoading = loading;
        emit isLoadingChanged();
    }
}

/**
 * @brief 设置最后错误
 */
void PasswordManager::setLastError(const QString &error)
{
    if (m_lastError != error) {
        m_lastError = error;
        emit lastErrorChanged();
    }
}

/**
 * @brief 清除最后错误
 */
void PasswordManager::clearLastError()
{
    setLastError("");
}

/**
 * @brief 解析CSV行
 */
QStringList PasswordManager::parseCsvLine(const QString &line)
{
    QStringList fields;
    QString currentField;
    bool inQuotes = false;
    
    for (int i = 0; i < line.length(); ++i) {
        QChar ch = line[i];
        
        if (ch == '"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                // 转义的引号
                currentField += '"';
                i++; // 跳过下一个引号
            } else {
                // 开始或结束引号
                inQuotes = !inQuotes;
            }
        } else if (ch == ',' && !inQuotes) {
            // 字段分隔符
            fields.append(currentField);
            currentField.clear();
        } else {
            currentField += ch;
        }
    }
    
    // 添加最后一个字段
    fields.append(currentField);
    
    return fields;
} 