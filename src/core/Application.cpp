#include "Application.h"
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include "../database/DatabaseManager.h"

// 静态成员初始化
Application* Application::s_instance = nullptr;

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
Application::Application(QObject *parent)
    : QObject(parent)
    , m_passwordManager(nullptr)
{
    s_instance = this;
    
    // 设置应用程序信息
    QCoreApplication::setApplicationName("QtSecretTool");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setOrganizationName("FlyingonTemp");
    QCoreApplication::setOrganizationDomain("github.com/FlyingonTemp");

    // 设置日志
    setupLogging();

    // 创建密码管理器
    m_passwordManager = new PasswordManager(this);

    qInfo() << "Application created";
}

/**
 * @brief 析构函数
 */
Application::~Application()
{
    qInfo() << "Application destroyed";
    s_instance = nullptr;
}

/**
 * @brief 获取应用程序版本
 * @return 版本字符串
 */
QString Application::version() const
{
    return QCoreApplication::applicationVersion();
}

/**
 * @brief 获取构建日期
 * @return 构建日期字符串
 */
QString Application::buildDate() const
{
    return QString("%1 %2").arg(__DATE__, __TIME__);
}

/**
 * @brief 初始化应用程序
 * @return 初始化是否成功
 */
bool Application::initialize()
{
    qInfo() << "Initializing application...";

    // 初始化数据库管理器
    DatabaseManager *dbManager = DatabaseManager::instance();
    if (!dbManager->initialize()) {
        qCritical() << "Failed to initialize database manager";
        return false;
    }

    // 初始化密码管理器
    if (!m_passwordManager->initialize()) {
        qCritical() << "Failed to initialize password manager";
        return false;
    }

    qInfo() << "Application initialized successfully";
    return true;
}

/**
 * @brief 退出应用程序
 */
void Application::quit()
{
    qInfo() << "Application quit requested";
    QCoreApplication::quit();
}

/**
 * @brief 获取应用程序实例
 * @return 应用程序实例指针
 */
Application* Application::instance()
{
    return s_instance;
}

/**
 * @brief 设置日志系统
 */
void Application::setupLogging()
{
    // 设置日志格式
    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] [%{category}] %{message}");

    // 创建日志目录
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    QDir().mkpath(logDir);

    qInfo() << "Logging setup completed";
} 