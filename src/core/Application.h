#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QQmlEngine>
#include "PasswordManager.h"

/**
 * @brief 应用程序主类
 * 
 * 管理整个应用程序的生命周期和核心组件
 * 提供QML可访问的应用程序级别功能
 */
class Application : public QObject
{
    Q_OBJECT

    // QML属性
    Q_PROPERTY(PasswordManager* passwordManager READ passwordManager CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString buildDate READ buildDate CONSTANT)

public:
    explicit Application(QObject *parent = nullptr);
    ~Application() override;

    // 属性访问方法
    PasswordManager* passwordManager() const { return m_passwordManager; }
    QString version() const;
    QString buildDate() const;

    // QML调用方法
    Q_INVOKABLE bool initialize();
    Q_INVOKABLE void quit();

    // 单例访问方法（用于C++代码）
    static Application* instance();

private:
    static Application *s_instance;
    PasswordManager *m_passwordManager;

    void setupLogging();
};

#endif // APPLICATION_H 