#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

#include <QObject>
#include <QQmlEngine>
#include "models/PasswordItem.h"
#include "models/PasswordListModel.h"
#include "database/DatabaseManager.h"

class QTimer;

/**
 * @brief 密码管理器核心类
 * 
 * 提供密码管理的核心业务逻辑，连接数据库和UI界面
 * 处理密码的增删改查、搜索、导入导出等功能
 */
class PasswordManager : public QObject
{
    Q_OBJECT

    // QML属性
    Q_PROPERTY(PasswordListModel* passwordListModel READ passwordListModel CONSTANT)
    Q_PROPERTY(bool isInitialized READ isInitialized NOTIFY initializedChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(int totalPasswordsCount READ totalPasswordsCount NOTIFY totalPasswordsCountChanged)

public:
    explicit PasswordManager(QObject *parent = nullptr);
    ~PasswordManager() override;

    // 属性访问方法
    PasswordListModel* passwordListModel() const { return m_passwordListModel; }
    bool isInitialized() const { return m_isInitialized; }
    bool isLoading() const { return m_isLoading; }
    QString lastError() const { return m_lastError; }
    int totalPasswordsCount() const;

    // QML调用的公共方法
    /**
     * @brief 初始化密码管理器
     * @param databasePath 数据库文件路径，为空则使用默认路径
     * @return 初始化是否成功
     */
    Q_INVOKABLE bool initialize(const QString &databasePath = QString());

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
    Q_INVOKABLE PasswordItem* createPasswordItem(const QString &title,
                                               const QString &username,
                                               const QString &password,
                                               const QString &website = QString(),
                                               const QString &notes = QString(),
                                               const QString &category = QString());

    /**
     * @brief 保存密码项目
     * @param item 要保存的密码项目
     * @return 保存是否成功
     */
    Q_INVOKABLE bool savePassword(PasswordItem *item);

    /**
     * @brief 更新密码项目
     * @param item 要更新的密码项目
     * @return 更新是否成功
     */
    Q_INVOKABLE bool updatePassword(PasswordItem *item);

    /**
     * @brief 删除密码项目
     * @param id 要删除的密码项目ID
     * @return 删除是否成功
     */
    Q_INVOKABLE bool deletePassword(int id);

    /**
     * @brief 根据ID获取密码项目
     * @param id 密码项目ID
     * @return 密码项目指针
     */
    Q_INVOKABLE PasswordItem* getPassword(int id);

    /**
     * @brief 生成随机密码
     * @param length 密码长度
     * @param includeSymbols 是否包含特殊字符
     * @return 生成的随机密码
     */
    Q_INVOKABLE QString generatePassword(int length = 12, bool includeSymbols = true);

    /**
     * @brief 搜索密码
     * @param searchTerm 搜索词
     */
    Q_INVOKABLE void searchPasswords(const QString &searchTerm);

    /**
     * @brief 获取所有分类
     * @return 分类列表
     */
    Q_INVOKABLE QStringList getCategories();

    /**
     * @brief 按分类过滤
     * @param category 分类名称
     */
    Q_INVOKABLE void filterByCategory(const QString &category);

    /**
     * @brief 显示收藏的密码
     * @param showFavoritesOnly 是否只显示收藏
     */
    Q_INVOKABLE void showFavoritesOnly(bool showFavoritesOnly);

    /**
     * @brief 清除所有过滤器
     */
    Q_INVOKABLE void clearFilters();

    /**
     * @brief 刷新密码列表
     */
    Q_INVOKABLE void refreshPasswordList();

    /**
     * @brief 清空所有密码
     * @return 清空是否成功
     */
    Q_INVOKABLE bool clearAllPasswords();

    /**
     * @brief 获取数据库统计信息
     * @return 统计信息Map
     */
    Q_INVOKABLE QVariantMap getDatabaseStats();

    /**
     * @brief 备份数据库
     * @param backupPath 备份文件路径
     * @return 备份是否成功
     */
    Q_INVOKABLE bool backupDatabase(const QString &backupPath);

    /**
     * @brief 恢复数据库
     * @param backupPath 备份文件路径
     * @return 恢复是否成功
     */
    Q_INVOKABLE bool restoreDatabase(const QString &backupPath);

    /**
     * @brief 导出密码数据为JSON格式
     * @param filePath 导出文件路径
     * @param includePasswords 是否包含密码明文
     * @return 导出是否成功
     */
    Q_INVOKABLE bool exportToJson(const QString &filePath, bool includePasswords = false);

    /**
     * @brief 从JSON文件导入密码数据
     * @param filePath 导入文件路径
     * @param mergeMode 是否为合并模式（true为合并，false为替换）
     * @return 导入是否成功
     */
    Q_INVOKABLE bool importFromJson(const QString &filePath, bool mergeMode = true);

    /**
     * @brief 导出密码数据为CSV格式
     * @param filePath 导出文件路径
     * @param includePasswords 是否包含密码明文
     * @return 导出是否成功
     */
    Q_INVOKABLE bool exportToCsv(const QString &filePath, bool includePasswords = false);

    /**
     * @brief 从CSV文件导入密码数据
     * @param filePath 导入文件路径
     * @param mergeMode 是否为合并模式
     * @return 导入是否成功
     */
    Q_INVOKABLE bool importFromCsv(const QString &filePath, bool mergeMode = true);

signals:
    void initializedChanged();
    void loadingChanged();
    void lastErrorChanged();
    void totalPasswordsCountChanged();
    
    void passwordAdded(PasswordItem *item);
    void passwordUpdated(PasswordItem *item);
    void passwordDeleted(int id);
    
    void operationCompleted(const QString &operation, bool success, const QString &message);
    void importProgressUpdated(int current, int total);

private slots:
    void onDatabaseError(const QString &error);
    void onPasswordItemSaved(PasswordItem *item);
    void onPasswordItemUpdated(PasswordItem *item);
    void onPasswordItemDeleted(int id);

private:
    DatabaseManager *m_databaseManager;        // 数据库管理器
    PasswordListModel *m_passwordListModel;    // 密码列表模型
    
    bool m_isInitialized;                      // 是否已初始化
    bool m_isLoading;                          // 是否正在加载
    QString m_lastError;                       // 最后的错误信息

    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void setLastError(const QString &error);

    /**
     * @brief 设置加载状态
     * @param loading 是否正在加载
     */
    void setLoading(bool loading);

    /**
     * @brief 从数据库加载所有密码
     */
    void loadPasswordsFromDatabase();

    /**
     * @brief 验证密码项目数据
     * @param item 要验证的密码项目
     * @return 验证结果
     */
    bool validatePasswordItem(PasswordItem *item) const;
};

#endif // PASSWORDMANAGER_H 