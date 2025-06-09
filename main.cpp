#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QtQml>
#include "src/core/Application.h"
#include "src/core/PasswordManager.h"
#include "src/models/PasswordItem.h"
#include "src/models/PasswordListModel.h"

int main(int argc, char *argv[])
{
    // 创建Qt应用程序
    QGuiApplication app(argc, argv);

    // 设置应用程序图标
    app.setWindowIcon(QIcon(":/icons/app_icon.png"));

    // 创建应用程序核心实例
    Application appCore;

    // 初始化应用程序
    if (!appCore.initialize()) {
        qCritical() << "Failed to initialize application";
        return -1;
    }

    // 创建QML引擎
    QQmlApplicationEngine engine;
    
    // 注册自定义类型到QML
    qmlRegisterSingletonInstance("QtSecretTool", 1, 0, "App", &appCore);
    qmlRegisterType<PasswordManager>("QtSecretTool", 1, 0, "PasswordManager");
    qmlRegisterType<PasswordItem>("QtSecretTool", 1, 0, "PasswordItem");
    qmlRegisterType<PasswordListModel>("QtSecretTool", 1, 0, "PasswordListModel");

    // 设置失败处理
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // 加载主QML文件
    engine.loadFromModule("QtSecretTool", "Main");

    return app.exec();
}
