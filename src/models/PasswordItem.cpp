#include "PasswordItem.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QUrl>
#include <QDebug>

/**
 * @brief 默认构造函数
 * @param parent 父对象指针
 */
PasswordItem::PasswordItem(QObject *parent)
    : QObject(parent)
    , m_id(-1)
    , m_isFavorite(false)
{
    // 设置创建时间和更新时间为当前时间
    m_createdAt = QDateTime::currentDateTime();
    m_updatedAt = m_createdAt;
}

/**
 * @brief 带参数构造函数
 * @param title 密码项目标题
 * @param username 用户名
 * @param password 密码
 * @param website 网站URL
 * @param notes 备注
 * @param category 分类
 * @param parent 父对象指针
 */
PasswordItem::PasswordItem(const QString &title, 
                          const QString &username, 
                          const QString &password,
                          const QString &website,
                          const QString &notes,
                          const QString &category,
                          QObject *parent)
    : QObject(parent)
    , m_id(-1)
    , m_title(title)
    , m_username(username)
    , m_password(password)
    , m_website(website)
    , m_notes(notes)
    , m_category(category)
    , m_isFavorite(false)
{
    // 设置创建时间和更新时间为当前时间
    m_createdAt = QDateTime::currentDateTime();
    m_updatedAt = m_createdAt;
}

// Setter方法的实现

void PasswordItem::setId(int id)
{
    if (m_id != id) {
        m_id = id;
        emit idChanged();
    }
}

void PasswordItem::setTitle(const QString &title)
{
    if (m_title != title) {
        m_title = title;
        updateTimestamp();
        emit titleChanged();
    }
}

void PasswordItem::setUsername(const QString &username)
{
    if (m_username != username) {
        m_username = username;
        updateTimestamp();
        emit usernameChanged();
    }
}

void PasswordItem::setPassword(const QString &password)
{
    if (m_password != password) {
        m_password = password;
        updateTimestamp();
        emit passwordChanged();
    }
}

void PasswordItem::setWebsite(const QString &website)
{
    if (m_website != website) {
        m_website = website;
        updateTimestamp();
        emit websiteChanged();
    }
}

void PasswordItem::setNotes(const QString &notes)
{
    if (m_notes != notes) {
        m_notes = notes;
        updateTimestamp();
        emit notesChanged();
    }
}

void PasswordItem::setCategory(const QString &category)
{
    if (m_category != category) {
        m_category = category;
        updateTimestamp();
        emit categoryChanged();
    }
}

void PasswordItem::setCreatedAt(const QDateTime &dateTime)
{
    if (m_createdAt != dateTime) {
        m_createdAt = dateTime;
        emit createdAtChanged();
    }
}

void PasswordItem::setUpdatedAt(const QDateTime &dateTime)
{
    if (m_updatedAt != dateTime) {
        m_updatedAt = dateTime;
        emit updatedAtChanged();
    }
}

void PasswordItem::setIsFavorite(bool favorite)
{
    if (m_isFavorite != favorite) {
        m_isFavorite = favorite;
        updateTimestamp();
        emit isFavoriteChanged();
    }
}

/**
 * @brief 验证密码项目数据是否有效
 * @return 如果标题和密码非空则返回true
 */
bool PasswordItem::isValid() const
{
    return !m_title.trimmed().isEmpty() && !m_password.isEmpty();
}

/**
 * @brief 为当前项目生成新密码
 * @param length 密码长度，默认12位
 * @param includeSymbols 是否包含特殊字符，默认true
 * @return 生成的密码字符串
 */
QString PasswordItem::generatePassword(int length, bool includeSymbols)
{
    QString newPassword = generateRandomPassword(length, includeSymbols);
    setPassword(newPassword);
    return newPassword;
}

/**
 * @brief 检查当前项目是否匹配搜索词
 * @param searchTerm 搜索词
 * @return 如果任何字段包含搜索词则返回true
 */
bool PasswordItem::matchesSearchTerm(const QString &searchTerm) const
{
    if (searchTerm.isEmpty()) {
        return true;
    }
    
    QString term = searchTerm.toLower();
    return m_title.toLower().contains(term) ||
           m_username.toLower().contains(term) ||
           m_website.toLower().contains(term) ||
           m_notes.toLower().contains(term) ||
           m_category.toLower().contains(term);
}

/**
 * @brief 获取网站的QUrl对象
 * @return 网站的QUrl对象，如果URL无效则返回空QUrl
 */
QUrl PasswordItem::getWebsiteUrl() const
{
    if (m_website.isEmpty()) {
        return QUrl();
    }
    
    QString url = m_website;
    // 如果URL不包含协议，添加https://
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        url.prepend("https://");
    }
    
    return QUrl(url);
}

/**
 * @brief 静态方法：生成随机密码
 * @param length 密码长度
 * @param includeSymbols 是否包含特殊字符
 * @return 生成的随机密码
 */
QString PasswordItem::generateRandomPassword(int length, bool includeSymbols)
{
    // 确保长度至少为4位
    length = qMax(4, length);
    
    QString lowercase = "abcdefghijklmnopqrstuvwxyz";
    QString uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QString numbers = "0123456789";
    QString symbols = "!@#$%^&*()_+-=[]{}|;:,.<>?";
    
    QString characters = lowercase + uppercase + numbers;
    if (includeSymbols) {
        characters += symbols;
    }
    
    QString password;
    QRandomGenerator *generator = QRandomGenerator::global();
    
    // 确保密码包含至少一个大写字母、小写字母和数字
    password += lowercase.at(generator->bounded(lowercase.length()));
    password += uppercase.at(generator->bounded(uppercase.length()));
    password += numbers.at(generator->bounded(numbers.length()));
    
    // 如果包含特殊字符，确保至少有一个
    if (includeSymbols && length > 3) {
        password += symbols.at(generator->bounded(symbols.length()));
    }
    
    // 填充剩余的字符
    int remainingLength = length - password.length();
    for (int i = 0; i < remainingLength; ++i) {
        password += characters.at(generator->bounded(characters.length()));
    }
    
    // 打乱密码字符顺序
    for (int i = 0; i < password.length(); ++i) {
        int randomIndex = generator->bounded(password.length());
        password[i] = password[randomIndex];
    }
    
    return password;
}

/**
 * @brief 更新时间戳为当前时间
 */
void PasswordItem::updateTimestamp()
{
    setUpdatedAt(QDateTime::currentDateTime());
} 