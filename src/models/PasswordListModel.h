#ifndef PASSWORDLISTMODEL_H
#define PASSWORDLISTMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>
#include "PasswordItem.h"

/**
 * @brief 密码列表模型类
 * 
 * 继承自QAbstractListModel，用于在QML界面中显示密码列表
 * 支持搜索、过滤、排序等功能
 */
class PasswordListModel : public QAbstractListModel
{
    Q_OBJECT

    // QML属性
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString searchFilter READ searchFilter WRITE setSearchFilter NOTIFY searchFilterChanged)
    Q_PROPERTY(QString categoryFilter READ categoryFilter WRITE setCategoryFilter NOTIFY categoryFilterChanged)
    Q_PROPERTY(bool showFavoritesOnly READ showFavoritesOnly WRITE setShowFavoritesOnly NOTIFY showFavoritesOnlyChanged)

public:
    /**
     * @brief 自定义角色枚举
     * 
     * 定义在QML中可以访问的数据角色
     */
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        UsernameRole,
        PasswordRole,
        WebsiteRole,
        NotesRole,
        CategoryRole,
        CreatedAtRole,
        UpdatedAtRole,
        IsFavoriteRole,
        PasswordItemRole  // 返回完整的PasswordItem对象
    };

    explicit PasswordListModel(QObject *parent = nullptr);

    // QAbstractListModel接口实现
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 属性访问方法
    int count() const;
    QString searchFilter() const { return m_searchFilter; }
    QString categoryFilter() const { return m_categoryFilter; }
    bool showFavoritesOnly() const { return m_showFavoritesOnly; }

    void setSearchFilter(const QString &filter);
    void setCategoryFilter(const QString &filter);
    void setShowFavoritesOnly(bool showFavoritesOnly);

    // QML调用方法
    Q_INVOKABLE void addPassword(PasswordItem *item);
    Q_INVOKABLE void removePassword(int index);
    Q_INVOKABLE void removePasswordById(int id);
    Q_INVOKABLE PasswordItem* getPassword(int index);
    Q_INVOKABLE PasswordItem* getPasswordById(int id);
    Q_INVOKABLE void updatePassword(int index, PasswordItem *item);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void refresh();

    // 搜索和过滤
    Q_INVOKABLE QList<PasswordItem*> search(const QString &searchTerm);
    Q_INVOKABLE QStringList getCategories() const;
    Q_INVOKABLE QList<PasswordItem*> getFavorites() const;

    // 排序方法
    Q_INVOKABLE void sortByTitle(bool ascending = true);
    Q_INVOKABLE void sortByCategory(bool ascending = true);
    Q_INVOKABLE void sortByCreatedDate(bool ascending = false);
    Q_INVOKABLE void sortByUpdatedDate(bool ascending = false);

    // 数据操作
    void setPasswordItems(const QList<PasswordItem*> &items);
    QList<PasswordItem*> getAllPasswords() const;
    QList<PasswordItem*> getFilteredPasswords() const;

signals:
    void countChanged();
    void searchFilterChanged();
    void categoryFilterChanged();
    void showFavoritesOnlyChanged();
    void passwordAdded(PasswordItem *item);
    void passwordRemoved(int id);
    void passwordUpdated(PasswordItem *item);

private slots:
    void onPasswordItemChanged();

private:
    QList<PasswordItem*> m_passwordItems;     // 所有密码项目
    QList<PasswordItem*> m_filteredItems;     // 过滤后的密码项目
    QString m_searchFilter;                    // 搜索过滤器
    QString m_categoryFilter;                  // 分类过滤器
    bool m_showFavoritesOnly;                 // 是否只显示收藏

    void applyFilters();                      // 应用过滤器
    bool matchesFilters(PasswordItem *item) const;  // 检查项目是否匹配过滤条件
    void connectPasswordItem(PasswordItem *item);   // 连接密码项目的信号
    void disconnectPasswordItem(PasswordItem *item); // 断开密码项目的信号连接
};

#endif // PASSWORDLISTMODEL_H 