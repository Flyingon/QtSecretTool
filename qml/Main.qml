import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtSecretTool 1.0
import "components"

ApplicationWindow {
    id: mainWindow
    
    width: 1200
    height: 800
    minimumWidth: 800
    minimumHeight: 600
    visible: true
    title: qsTr("Qt Secret Tool - 密码管理器")

    property bool isLoading: App.passwordManager.isLoading

    // 密码管理器
    PasswordManager {
        id: passwordManager
        onMasterPasswordSet: {
            console.log("Master password set successfully")
            // 可以在这里添加成功提示
        }
        onMasterPasswordVerified: {
            console.log("Master password verified successfully")
            // 验证成功后可以继续应用启动
        }
        onMasterPasswordChanged: {
            console.log("Master password changed successfully")
            // 可以在这里添加成功提示
        }
        onPasswordError: function(error) {
            console.log("Password error:", error)
            // 可以在这里添加错误提示
        }
    }
    
    // 启动时的主密码验证对话框
    MasterPasswordDialog {
        id: startupPasswordDialog
        visible: false
        onPasswordSet: {
            console.log("Startup password set")
            // 密码设置成功，刷新密码列表
            App.passwordManager.refreshPasswordList()
        }
        onPasswordChanged: {
            console.log("Startup password changed")
        }
        onPasswordVerified: {
            console.log("Startup password verified")
            // 密码验证成功，刷新密码列表
            App.passwordManager.refreshPasswordList()
        }
    }
    
    // 启动时检查主密码
    Component.onCompleted: {
        if (passwordManager.hasMasterPassword()) {
            // 如果已设置主密码，需要验证
            startupPasswordDialog.isFirstTime = false
            startupPasswordDialog.isChangingPassword = false
            startupPasswordDialog.reset()
            startupPasswordDialog.open()
        } else {
            // 如果未设置主密码，需要设置
            startupPasswordDialog.isFirstTime = true
            startupPasswordDialog.isChangingPassword = false
            startupPasswordDialog.reset()
            startupPasswordDialog.open()
        }
    }

    // 主要布局
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 左侧边栏
        Rectangle {
            Layout.preferredWidth: 250
            Layout.fillHeight: true
            color: "#f5f5f5"
            border.color: "#e0e0e0"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                // 搜索框
                SearchBar {
                    id: searchBar
                    Layout.fillWidth: true
                    placeholderText: qsTr("搜索密码...")
                    onTextChanged: {
                        App.passwordManager.searchPasswords(text)
                    }
                }

                // 快捷按钮
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    CustomButton {
                        Layout.fillWidth: true
                        text: qsTr("添加密码")
                        onClicked: {
                            addEditPasswordPage.openForAdd()
                            stackLayout.currentIndex = 1
                        }
                    }

                    CustomButton {
                        Layout.fillWidth: true
                        text: qsTr("收藏夹")
                        onClicked: {
                            App.passwordManager.showFavoritesOnly(true)
                        }
                    }

                    CustomButton {
                        Layout.fillWidth: true
                        text: qsTr("全部密码")
                        onClicked: {
                            App.passwordManager.clearFilters()
                        }
                    }
                }

                // 分类列表
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "white"
                    border.color: "#e0e0e0"
                    border.width: 1
                    radius: 4

                    ListView {
                        id: categoryListView
                        anchors.fill: parent
                        anchors.margins: 5
                        model: App.passwordManager.getCategories()

                        delegate: ItemDelegate {
                            width: categoryListView.width
                            height: 35

                            Text {
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.leftMargin: 10
                                text: modelData
                                color: "#333"
                            }

                            onClicked: {
                                App.passwordManager.filterByCategory(modelData)
                            }
                        }
                    }
                }

                // 统计信息
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    color: "#e8f4f8"
                    border.color: "#b3d9e6"
                    border.width: 1
                    radius: 4

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 2

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: qsTr("总计密码")
                            font.pointSize: 10
                            color: "#666"
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: App.passwordManager.totalPasswordsCount
                            font.pointSize: 16
                            font.bold: true
                            color: "#2196F3"
                        }
                    }
                }
            }
        }

        // 主内容区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"

            StackLayout {
                id: stackLayout
                anchors.fill: parent
                currentIndex: 0

                // 密码列表页面
                PasswordListPage {
                    id: passwordListPage
                    
                    onEditPasswordRequested: {
                        addEditPasswordPage.openForEdit(passwordItem)
                        stackLayout.currentIndex = 1
                    }
                }

                // 添加/编辑密码页面
                AddEditPasswordPage {
                    id: addEditPasswordPage
                    
                    onPasswordSaved: {
                        // 密码保存成功，返回列表页面
                        stackLayout.currentIndex = 0
                        App.passwordManager.refreshPasswordList()
                    }
                    
                    onCancelled: {
                        // 取消操作，返回列表页面
                        stackLayout.currentIndex = 0
                    }
                }

                // 设置页面
                SettingsPage {
                    id: settingsPage
                }
            }
        }
    }

    // 顶部菜单栏
    menuBar: MenuBar {
        Menu {
            title: qsTr("文件")
            
            MenuItem {
                text: qsTr("导入")
                onTriggered: {
                    stackLayout.currentIndex = 2  // 切换到设置页面
                }
            }
            
            MenuItem {
                text: qsTr("导出")
                onTriggered: {
                    stackLayout.currentIndex = 2  // 切换到设置页面
                }
            }
            
            MenuSeparator { }
            
            MenuItem {
                text: qsTr("退出")
                onTriggered: {
                    App.quit()
                }
            }
        }

        Menu {
            title: qsTr("编辑")
            
            MenuItem {
                text: qsTr("添加密码")
                onTriggered: {
                    addEditPasswordPage.openForAdd()
                    stackLayout.currentIndex = 1
                }
            }
        }

        Menu {
            title: qsTr("工具")
            
            MenuItem {
                text: qsTr("生成密码")
                onTriggered: {
                    passwordGeneratorDialog.open()
                }
            }
            
            MenuItem {
                text: qsTr("设置")
                onTriggered: {
                    stackLayout.currentIndex = 2
                }
            }
        }

        Menu {
            title: qsTr("帮助")
            
            MenuItem {
                text: qsTr("关于")
                onTriggered: {
                    // TODO: 显示关于对话框
                }
            }
        }
    }

    // 状态栏
    footer: Rectangle {
        height: 30
        color: "#f5f5f5"
        border.color: "#e0e0e0"
        border.width: 1
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: 5

            Text {
                text: App.passwordManager.lastError || qsTr("就绪")
                color: App.passwordManager.lastError ? "red" : "#666"
                font.pointSize: 9
            }

            Item { Layout.fillWidth: true }

            // 加载指示器
            BusyIndicator {
                visible: isLoading
                running: isLoading
                Layout.preferredWidth: 20
                Layout.preferredHeight: 20
            }

            Text {
                text: qsTr("版本: ") + App.version
                color: "#666"
                font.pointSize: 9
            }
        }
    }

    // 错误消息提示
    Rectangle {
        id: errorMessage
        visible: false
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 40
        color: "#ffebee"
        border.color: "#f44336"
        border.width: 1
        z: 100

        Text {
            anchors.centerIn: parent
            text: App.passwordManager.lastError
            color: "#d32f2f"
        }

        Timer {
            id: hideErrorTimer
            interval: 5000
            onTriggered: errorMessage.visible = false
        }
    }

    // 密码生成器对话框
    PasswordGeneratorDialog {
        id: passwordGeneratorDialog
        anchors.centerIn: parent
        
        onPasswordAccepted: {
            // 可以在这里处理生成的密码，比如复制到剪贴板
            console.log("Generated password:", password)
        }
    }

    // 监听错误变化
    Connections {
        target: App.passwordManager
        function onLastErrorChanged() {
            if (App.passwordManager.lastError) {
                errorMessage.visible = true
                hideErrorTimer.restart()
            }
        }
    }
}
