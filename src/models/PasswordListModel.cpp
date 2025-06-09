#include "PasswordListModel.h"
#include <QDebug>
#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
PasswordListModel::PasswordListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_showFavoritesOnly(false)
{
}

/**
 * @brief 返回模型中的行数
 * @param parent 父索引（在列表模型中通常忽略）
 * @return 过滤后的密码项目数量
 */
int PasswordListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_filteredItems.size();
}

/**
 * @brief 返回指定索引和角色的数据
 * @param index 模型索引
 * @param role 数据角色
 * @return 对应的数据
 */
QVariant PasswordListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_filteredItems.size()) {
        return QVariant();
    }

    PasswordItem *item = m_filteredItems.at(index.row());
    if (!item) {
        return QVariant();
    }

    switch (role) {
    case IdRole:
        return item->id();
    case TitleRole:
        return item->title();
    case UsernameRole:
        return item->username();
    case PasswordRole:
        return item->password();
    case WebsiteRole:
        return item->website();
    case NotesRole:
        return item->notes();
    case CategoryRole:
        return item->category();
    case CreatedAtRole:
        return item->createdAt();
    case UpdatedAtRole:
        return item->updatedAt();
    case IsFavoriteRole:
        return item->isFavorite();
    case PasswordItemRole:
        return QVariant::fromValue(item);
    default:
        return QVariant();
    }
}

/**
 * @brief 返回角色名称映射
 * @return 角色ID到角色名称的映射
 */
QHash<int, QByteArray> PasswordListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[UsernameRole] = "username";
    roles[PasswordRole] = "password";
    roles[WebsiteRole] = "website";
    roles[NotesRole] = "notes";
    roles[CategoryRole] = "category";
    roles[CreatedAtRole] = "createdAt";
    roles[UpdatedAtRole] = "updatedAt";
    roles[IsFavoriteRole] = "isFavorite";
    roles[PasswordItemRole] = "passwordItem";
    return roles;
}

/**
 * @brief 返回密码项目数量
 * @return 过滤后的密码项目数量
 */
int PasswordListModel::count() const
{
    return m_filteredItems.size();
}

/**
 * @brief 设置搜索过滤器
 * @param filter 搜索关键词
 */
void PasswordListModel::setSearchFilter(const QString &filter)
{
    if (m_searchFilter != filter) {
        m_searchFilter = filter;
        applyFilters();
        emit searchFilterChanged();
    }
}

/**
 * @brief 设置分类过滤器
 * @param filter 分类名称
 */
void PasswordListModel::setCategoryFilter(const QString &filter)
{
    if (m_categoryFilter != filter) {
        m_categoryFilter = filter;
        applyFilters();
        emit categoryFilterChanged();
    }
}

/**
 * @brief 设置是否只显示收藏项目
 * @param showFavoritesOnly 是否只显示收藏
 */
void PasswordListModel::setShowFavoritesOnly(bool showFavoritesOnly)
{
    if (m_showFavoritesOnly != showFavoritesOnly) {
        m_showFavoritesOnly = showFavoritesOnly;
        applyFilters();
        emit showFavoritesOnlyChanged();
    }
}

/**
 * @brief 添加密码项目
 * @param item 要添加的密码项目
 */
void PasswordListModel::addPassword(PasswordItem *item)
{
    if (!item) {
        return;
    }

    // 检查是否已存在相同ID的项目
    for (PasswordItem *existingItem : m_passwordItems) {
        if (existingItem->id() == item->id() && item->id() != -1) {
            qWarning() << "Password item with ID" << item->id() << "already exists";
            return;
        }
    }

    m_passwordItems.append(item);
    connectPasswordItem(item);
    applyFilters();
    emit passwordAdded(item);
}

/**
 * @brief 根据索引移除密码项目
 * @param index 要移除的项目在过滤列表中的索引
 */
void PasswordListModel::removePassword(int index)
{
    if (index < 0 || index >= m_filteredItems.size()) {
        return;
    }

    PasswordItem *item = m_filteredItems.at(index);
    removePasswordById(item->id());
}

/**
 * @brief 根据ID移除密码项目
 * @param id 要移除的密码项目ID
 */
void PasswordListModel::removePasswordById(int id)
{
    for (int i = 0; i < m_passwordItems.size(); ++i) {
        if (m_passwordItems.at(i)->id() == id) {
            PasswordItem *item = m_passwordItems.at(i);
            disconnectPasswordItem(item);
            m_passwordItems.removeAt(i);
            applyFilters();
            emit passwordRemoved(id);
            break;
        }
    }
}

/**
 * @brief 根据过滤列表索引获取密码项目
 * @param index 过滤列表中的索引
 * @return 密码项目指针，如果索引无效则返回nullptr
 */
PasswordItem* PasswordListModel::getPassword(int index)
{
    if (index < 0 || index >= m_filteredItems.size()) {
        return nullptr;
    }
    return m_filteredItems.at(index);
}

/**
 * @brief 根据ID获取密码项目
 * @param id 密码项目ID
 * @return 密码项目指针，如果未找到则返回nullptr
 */
PasswordItem* PasswordListModel::getPasswordById(int id)
{
    for (PasswordItem *item : m_passwordItems) {
        if (item->id() == id) {
            return item;
        }
    }
    return nullptr;
}

/**
 * @brief 更新密码项目
 * @param index 过滤列表中的索引
 * @param item 新的密码项目数据
 */
void PasswordListModel::updatePassword(int index, PasswordItem *item)
{
    if (!item || index < 0 || index >= m_filteredItems.size()) {
        return;
    }

    PasswordItem *existingItem = m_filteredItems.at(index);
    if (existingItem) {
        // 更新现有项目的数据
        existingItem->setTitle(item->title());
        existingItem->setUsername(item->username());
        existingItem->setPassword(item->password());
        existingItem->setWebsite(item->website());
        existingItem->setNotes(item->notes());
        existingItem->setCategory(item->category());
        existingItem->setIsFavorite(item->isFavorite());
        
        // 重新应用过滤器（因为可能影响过滤结果）
        applyFilters();
        emit passwordUpdated(existingItem);
    }
}

/**
 * @brief 清空所有密码项目
 */
void PasswordListModel::clear()
{
    beginResetModel();
    
    // 断开所有信号连接
    for (PasswordItem *item : m_passwordItems) {
        disconnectPasswordItem(item);
    }
    
    m_passwordItems.clear();
    m_filteredItems.clear();
    endResetModel();
    emit countChanged();
}

/**
 * @brief 刷新模型数据
 */
void PasswordListModel::refresh()
{
    applyFilters();
}

/**
 * @brief 搜索密码项目
 * @param searchTerm 搜索词
 * @return 匹配的密码项目列表
 */
QList<PasswordItem*> PasswordListModel::search(const QString &searchTerm)
{
    QList<PasswordItem*> results;
    for (PasswordItem *item : m_passwordItems) {
        if (item->matchesSearchTerm(searchTerm)) {
            results.append(item);
        }
    }
    return results;
}

/**
 * @brief 获取所有分类
 * @return 分类名称列表
 */
QStringList PasswordListModel::getCategories() const
{
    QStringList categories;
    for (PasswordItem *item : m_passwordItems) {
        QString category = item->category();
        if (!category.isEmpty() && !categories.contains(category)) {
            categories.append(category);
        }
    }
    categories.sort();
    return categories;
}

/**
 * @brief 获取所有收藏项目
 * @return 收藏的密码项目列表
 */
QList<PasswordItem*> PasswordListModel::getFavorites() const
{
    QList<PasswordItem*> favorites;
    for (PasswordItem *item : m_passwordItems) {
        if (item->isFavorite()) {
            favorites.append(item);
        }
    }
    return favorites;
}

/**
 * @brief 按标题排序
 * @param ascending 是否升序排列
 */
void PasswordListModel::sortByTitle(bool ascending)
{
    beginResetModel();
    std::sort(m_filteredItems.begin(), m_filteredItems.end(),
              [ascending](const PasswordItem *a, const PasswordItem *b) {
                  return ascending ? a->title() < b->title() : a->title() > b->title();
              });
    endResetModel();
}

/**
 * @brief 按分类排序
 * @param ascending 是否升序排列
 */
void PasswordListModel::sortByCategory(bool ascending)
{
    beginResetModel();
    std::sort(m_filteredItems.begin(), m_filteredItems.end(),
              [ascending](const PasswordItem *a, const PasswordItem *b) {
                  return ascending ? a->category() < b->category() : a->category() > b->category();
              });
    endResetModel();
}

/**
 * @brief 按创建日期排序
 * @param ascending 是否升序排列（默认false，即最新的在前）
 */
void PasswordListModel::sortByCreatedDate(bool ascending)
{
    beginResetModel();
    std::sort(m_filteredItems.begin(), m_filteredItems.end(),
              [ascending](const PasswordItem *a, const PasswordItem *b) {
                  return ascending ? a->createdAt() < b->createdAt() : a->createdAt() > b->createdAt();
              });
    endResetModel();
}

/**
 * @brief 按更新日期排序
 * @param ascending 是否升序排列（默认false，即最新更新的在前）
 */
void PasswordListModel::sortByUpdatedDate(bool ascending)
{
    beginResetModel();
    std::sort(m_filteredItems.begin(), m_filteredItems.end(),
              [ascending](const PasswordItem *a, const PasswordItem *b) {
                  return ascending ? a->updatedAt() < b->updatedAt() : a->updatedAt() > b->updatedAt();
              });
    endResetModel();
}

/**
 * @brief 设置密码项目列表
 * @param items 新的密码项目列表
 */
void PasswordListModel::setPasswordItems(const QList<PasswordItem*> &items)
{
    beginResetModel();
    
    // 断开旧的信号连接
    for (PasswordItem *item : m_passwordItems) {
        disconnectPasswordItem(item);
    }
    
    m_passwordItems = items;
    
    // 连接新的信号
    for (PasswordItem *item : m_passwordItems) {
        connectPasswordItem(item);
    }
    
    applyFilters();
    endResetModel();
    emit countChanged();
}

/**
 * @brief 获取所有密码项目
 * @return 所有密码项目列表
 */
QList<PasswordItem*> PasswordListModel::getAllPasswords() const
{
    return m_passwordItems;
}

/**
 * @brief 获取过滤后的密码项目
 * @return 过滤后的密码项目列表
 */
QList<PasswordItem*> PasswordListModel::getFilteredPasswords() const
{
    return m_filteredItems;
}

/**
 * @brief 密码项目数据变化时的槽函数
 */
void PasswordListModel::onPasswordItemChanged()
{
    // 重新应用过滤器，因为数据变化可能影响过滤结果
    applyFilters();
    
    // 查找变化的项目并发出信号
    PasswordItem *changedItem = qobject_cast<PasswordItem*>(sender());
    if (changedItem) {
        emit passwordUpdated(changedItem);
    }
}

/**
 * @brief 应用所有过滤器
 */
void PasswordListModel::applyFilters()
{
    beginResetModel();
    
    m_filteredItems.clear();
    for (PasswordItem *item : m_passwordItems) {
        if (matchesFilters(item)) {
            m_filteredItems.append(item);
        }
    }
    
    endResetModel();
    emit countChanged();
}

/**
 * @brief 检查项目是否匹配过滤条件
 * @param item 要检查的密码项目
 * @return 如果匹配所有过滤条件则返回true
 */
bool PasswordListModel::matchesFilters(PasswordItem *item) const
{
    if (!item) {
        return false;
    }
    
    // 搜索过滤器
    if (!m_searchFilter.isEmpty() && !item->matchesSearchTerm(m_searchFilter)) {
        return false;
    }
    
    // 分类过滤器
    if (!m_categoryFilter.isEmpty() && item->category() != m_categoryFilter) {
        return false;
    }
    
    // 收藏过滤器
    if (m_showFavoritesOnly && !item->isFavorite()) {
        return false;
    }
    
    return true;
}

/**
 * @brief 连接密码项目的信号
 * @param item 要连接信号的密码项目
 */
void PasswordListModel::connectPasswordItem(PasswordItem *item)
{
    if (!item) {
        return;
    }
    
    // 连接所有变化信号到槽函数
    connect(item, &PasswordItem::titleChanged, this, &PasswordListModel::onPasswordItemChanged);
    connect(item, &PasswordItem::usernameChanged, this, &PasswordListModel::onPasswordItemChanged);
    connect(item, &PasswordItem::passwordChanged, this, &PasswordListModel::onPasswordItemChanged);
    connect(item, &PasswordItem::websiteChanged, this, &PasswordListModel::onPasswordItemChanged);
    connect(item, &PasswordItem::notesChanged, this, &PasswordListModel::onPasswordItemChanged);
    connect(item, &PasswordItem::categoryChanged, this, &PasswordListModel::onPasswordItemChanged);
    connect(item, &PasswordItem::isFavoriteChanged, this, &PasswordListModel::onPasswordItemChanged);
}

/**
 * @brief 断开密码项目的信号连接
 * @param item 要断开信号连接的密码项目
 */
void PasswordListModel::disconnectPasswordItem(PasswordItem *item)
{
    if (!item) {
        return;
    }
    
    // 断开所有信号连接
    disconnect(item, nullptr, this, nullptr);
} 