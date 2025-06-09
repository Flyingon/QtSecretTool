#ifndef PASSWORDITEM_H
#define PASSWORDITEM_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QUrl>

/**
 * @brief 密码项目数据模型类
 * 
 * 用于表示单个密码条目的数据结构，包含所有必要的字段
 * 支持增删改查操作和数据验证
 */
class PasswordItem : public QObject
{
    Q_OBJECT

    // QML属性绑定
    Q_PROPERTY(int id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(QString website READ website WRITE setWebsite NOTIFY websiteChanged)
    Q_PROPERTY(QString notes READ notes WRITE setNotes NOTIFY notesChanged)
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
    Q_PROPERTY(QDateTime createdAt READ createdAt WRITE setCreatedAt NOTIFY createdAtChanged)
    Q_PROPERTY(QDateTime updatedAt READ updatedAt WRITE setUpdatedAt NOTIFY updatedAtChanged)
    Q_PROPERTY(bool isFavorite READ isFavorite WRITE setIsFavorite NOTIFY isFavoriteChanged)

public:
    explicit PasswordItem(QObject *parent = nullptr);
    
    // 构造函数重载 - 用于创建新的密码项目
    PasswordItem(const QString &title, 
                const QString &username, 
                const QString &password,
                const QString &website = QString(),
                const QString &notes = QString(),
                const QString &category = QString(),
                QObject *parent = nullptr);

    // Getter方法
    int id() const { return m_id; }
    QString title() const { return m_title; }
    QString username() const { return m_username; }
    QString password() const { return m_password; }
    QString website() const { return m_website; }
    QString notes() const { return m_notes; }
    QString category() const { return m_category; }
    QDateTime createdAt() const { return m_createdAt; }
    QDateTime updatedAt() const { return m_updatedAt; }
    bool isFavorite() const { return m_isFavorite; }

    // Setter方法
    void setId(int id);
    void setTitle(const QString &title);
    void setUsername(const QString &username);
    void setPassword(const QString &password);
    void setWebsite(const QString &website);
    void setNotes(const QString &notes);
    void setCategory(const QString &category);
    void setCreatedAt(const QDateTime &dateTime);
    void setUpdatedAt(const QDateTime &dateTime);
    void setIsFavorite(bool favorite);

    // 工具方法
    Q_INVOKABLE bool isValid() const;
    Q_INVOKABLE QString generatePassword(int length = 12, bool includeSymbols = true);
    Q_INVOKABLE bool matchesSearchTerm(const QString &searchTerm) const;
    Q_INVOKABLE QUrl getWebsiteUrl() const;

    // 静态工具方法
    static QString generateRandomPassword(int length = 12, bool includeSymbols = true);

signals:
    void idChanged();
    void titleChanged();
    void usernameChanged();
    void passwordChanged();
    void websiteChanged();
    void notesChanged();
    void categoryChanged();
    void createdAtChanged();
    void updatedAtChanged();
    void isFavoriteChanged();

private:
    int m_id;                  // 唯一ID，用于数据库主键
    QString m_title;           // 密码项目标题
    QString m_username;        // 用户名/邮箱
    QString m_password;        // 密码（将在存储时加密）
    QString m_website;         // 网站URL
    QString m_notes;           // 备注信息
    QString m_category;        // 分类（如：社交媒体、工作、银行等）
    QDateTime m_createdAt;     // 创建时间
    QDateTime m_updatedAt;     // 最后更新时间
    bool m_isFavorite;         // 是否为收藏项目

    void updateTimestamp();    // 更新时间戳的私有方法
};

#endif // PASSWORDITEM_H 