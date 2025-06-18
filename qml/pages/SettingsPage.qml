import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import QtSecretTool 1.0

Rectangle {
    id: settingsPage
    color: "white"
    
    property bool hasUnsavedChanges: false
    
    // 密码管理器
    PasswordManager {
        id: passwordManager
        onMasterPasswordSet: {
            console.log("Master password set successfully")
            // 可以在这里添加成功提示
        }
        onMasterPasswordVerified: {
            console.log("Master password verified successfully")
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
    
    // 主密码对话框
    MasterPasswordDialog {
        id: masterPasswordDialog
        onPasswordSet: {
            console.log("Password set from dialog")
        }
        onPasswordChanged: {
            console.log("Password changed from dialog")
        }
    }
    
    ScrollView {
        anchors.fill: parent
        anchors.margins: 20
        
        ColumnLayout {
            width: Math.min(800, settingsPage.width - 40)
            spacing: 20
            
            // 标题
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("设置")
                font.pointSize: 24
                font.bold: true
                color: "#333"
            }
            
            // 安全设置
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("安全设置")
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 15
                    
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Text {
                            text: qsTr("主密码:")
                            color: "#333"
                            Layout.preferredWidth: 120
                        }
                        
                        Button {
                            text: passwordManager.hasMasterPassword() ? qsTr("更改主密码") : qsTr("设置主密码")
                            onClicked: {
                                masterPasswordDialog.isFirstTime = !passwordManager.hasMasterPassword()
                                masterPasswordDialog.isChangingPassword = passwordManager.hasMasterPassword()
                                masterPasswordDialog.reset()
                                masterPasswordDialog.open()
                            }
                        }
                        
                        Item { Layout.fillWidth: true }
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Text {
                            text: qsTr("自动锁定:")
                            color: "#333"
                            Layout.preferredWidth: 120
                        }
                        
                        ComboBox {
                            id: autoLockComboBox
                            model: [
                                qsTr("从不"),
                                qsTr("5分钟"),
                                qsTr("15分钟"),
                                qsTr("30分钟"),
                                qsTr("1小时")
                            ]
                            currentIndex: 2
                        }
                        
                        Item { Layout.fillWidth: true }
                    }
                    
                    CheckBox {
                        text: qsTr("启动时需要验证主密码")
                        checked: true
                    }
                    
                    CheckBox {
                        text: qsTr("复制密码后自动清除剪贴板")
                        checked: true
                    }
                }
            }
            
            // 数据管理
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("数据管理")
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 15
                    
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Text {
                            text: qsTr("数据库路径:")
                            color: "#333"
                            Layout.preferredWidth: 120
                        }
                        
                        TextField {
                            Layout.fillWidth: true
                            text: "/Users/yuanzhaoyi/Library/Application Support/FlyingonTemp/QtSecretTool/passwords.db"
                            readOnly: true
                            selectByMouse: true
                        }
                        
                        Button {
                            text: qsTr("更改")
                            enabled: false  // 暂时禁用
                        }
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        
                        Button {
                            text: qsTr("备份数据")
                            onClicked: backupFileDialog.open()
                        }
                        
                        Button {
                            text: qsTr("恢复数据")
                            onClicked: restoreFileDialog.open()
                        }
                        
                        Button {
                            text: qsTr("清除所有数据")
                            enabled: false  // 需要确认对话框
                            background: Rectangle {
                                color: parent.enabled ? (parent.hovered ? "#d32f2f" : "#f44336") : "#e0e0e0"
                                border.color: parent.enabled ? "#d32f2f" : "#bdbdbd"
                                border.width: 1
                                radius: 4
                            }
                            contentItem: Text {
                                text: parent.text
                                color: parent.enabled ? "white" : "#9e9e9e"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                clearDataConfirmDialog.open()
                            }
                        }
                        
                        Item { Layout.fillWidth: true }
                    }
                }
            }
            
            // 导入导出
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("导入导出")
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 15
                    
                    Text {
                        text: qsTr("支持的格式：JSON、CSV")
                        color: "#666"
                        font.pointSize: 10
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        
                        Button {
                            text: qsTr("导出为JSON")
                            onClicked: exportJsonFileDialog.open()
                        }
                        
                        Button {
                            text: qsTr("导出为CSV")
                            onClicked: exportCsvFileDialog.open()
                        }
                        
                        Button {
                            text: qsTr("从JSON导入")
                            onClicked: importJsonFileDialog.open()
                        }
                        
                        Button {
                            text: qsTr("从CSV导入")
                            onClicked: importCsvFileDialog.open()
                        }
                        
                        Item { Layout.fillWidth: true }
                    }
                    
                    CheckBox {
                        text: qsTr("导出时包含密码明文（不推荐）")
                        id: includePasswordsCheckBox
                    }
                }
            }
            
            // 应用信息
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("应用信息")
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Text {
                            text: qsTr("版本:")
                            color: "#333"
                            Layout.preferredWidth: 80
                        }
                        
                        Text {
                            text: App.version
                            color: "#666"
                        }
                        
                        Item { Layout.fillWidth: true }
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Text {
                            text: qsTr("构建日期:")
                            color: "#333"
                            Layout.preferredWidth: 80
                        }
                        
                        Text {
                            text: App.buildDate
                            color: "#666"
                        }
                        
                        Item { Layout.fillWidth: true }
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Text {
                            text: qsTr("密码数量:")
                            color: "#333"
                            Layout.preferredWidth: 80
                        }
                        
                        Text {
                            text: App.passwordManager.totalPasswordsCount
                            color: "#666"
                        }
                        
                        Item { Layout.fillWidth: true }
                    }
                }
            }
            
            // 底部间距
            Item {
                Layout.fillHeight: true
                Layout.minimumHeight: 20
            }
        }
    }
    
    // 文件对话框
    FileDialog {
        id: backupFileDialog
        title: qsTr("选择备份保存位置")
        fileMode: FileDialog.SaveFile
        nameFilters: [qsTr("数据库文件 (*.db)"), qsTr("所有文件 (*)")]
        defaultSuffix: "db"
        onAccepted: {
            if (App.passwordManager.backupDatabase(selectedFile)) {
                statusMessage.showMessage(qsTr("备份成功"), false)
            } else {
                statusMessage.showMessage(qsTr("备份失败: ") + App.passwordManager.lastError, true)
            }
        }
    }
    
    FileDialog {
        id: restoreFileDialog
        title: qsTr("选择要恢复的备份文件")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("数据库文件 (*.db)"), qsTr("所有文件 (*)")]
        onAccepted: {
            if (App.passwordManager.restoreDatabase(selectedFile)) {
                statusMessage.showMessage(qsTr("恢复成功"), false)
            } else {
                statusMessage.showMessage(qsTr("恢复失败: ") + App.passwordManager.lastError, true)
            }
        }
    }
    
    FileDialog {
        id: exportJsonFileDialog
        title: qsTr("导出为JSON文件")
        fileMode: FileDialog.SaveFile
        nameFilters: [qsTr("JSON文件 (*.json)"), qsTr("所有文件 (*)")]
        defaultSuffix: "json"
        onAccepted: {
            if (App.passwordManager.exportToJson(selectedFile, includePasswordsCheckBox.checked)) {
                statusMessage.showMessage(qsTr("导出成功"), false)
            } else {
                statusMessage.showMessage(qsTr("导出失败: ") + App.passwordManager.lastError, true)
            }
        }
    }
    
    FileDialog {
        id: exportCsvFileDialog
        title: qsTr("导出为CSV文件")
        fileMode: FileDialog.SaveFile
        nameFilters: [qsTr("CSV文件 (*.csv)"), qsTr("所有文件 (*)")]
        defaultSuffix: "csv"
        onAccepted: {
            if (App.passwordManager.exportToCsv(selectedFile, includePasswordsCheckBox.checked)) {
                statusMessage.showMessage(qsTr("导出成功"), false)
            } else {
                statusMessage.showMessage(qsTr("导出失败: ") + App.passwordManager.lastError, true)
            }
        }
    }
    
    FileDialog {
        id: importJsonFileDialog
        title: qsTr("选择要导入的JSON文件")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("JSON文件 (*.json)"), qsTr("所有文件 (*)")]
        onAccepted: {
            if (App.passwordManager.importFromJson(selectedFile, true)) {
                statusMessage.showMessage(qsTr("导入成功"), false)
            } else {
                statusMessage.showMessage(qsTr("导入失败: ") + App.passwordManager.lastError, true)
            }
        }
    }
    
    FileDialog {
        id: importCsvFileDialog
        title: qsTr("选择要导入的CSV文件")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("CSV文件 (*.csv)"), qsTr("所有文件 (*)")]
        onAccepted: {
            if (App.passwordManager.importFromCsv(selectedFile, true)) {
                statusMessage.showMessage(qsTr("导入成功"), false)
            } else {
                statusMessage.showMessage(qsTr("导入失败: ") + App.passwordManager.lastError, true)
            }
        }
    }
    
    // 清除数据确认对话框
    Dialog {
        id: clearDataConfirmDialog
        anchors.centerIn: parent
        width: 400
        height: 180
        title: qsTr("确认清除数据")
        modal: true
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 15
            
            Text {
                Layout.fillWidth: true
                text: qsTr("警告：此操作将删除所有保存的密码数据，且不可恢复！\n\n请确认您已经备份了重要数据。")
                wrapMode: Text.WordWrap
                color: "#d32f2f"
                font.bold: true
            }
            
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 10
                
                Item { Layout.fillWidth: true }
                
                Button {
                    text: qsTr("取消")
                    onClicked: clearDataConfirmDialog.close()
                }
                
                Button {
                    text: qsTr("确认清除")
                    background: Rectangle {
                        color: parent.hovered ? "#d32f2f" : "#f44336"
                        border.color: "#d32f2f"
                        border.width: 1
                        radius: 4
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        if (App.passwordManager.clearAllPasswords()) {
                            statusMessage.showMessage(qsTr("数据已清除"), false)
                        } else {
                            statusMessage.showMessage(qsTr("清除失败: ") + App.passwordManager.lastError, true)
                        }
                        clearDataConfirmDialog.close()
                    }
                }
            }
        }
    }
    
    // 状态消息
    Rectangle {
        id: statusMessage
        visible: false
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 40
        color: "#4caf50"
        border.color: "#388e3c"
        border.width: 1
        z: 100
        
        property alias text: statusText.text
        property bool isError: false
        
        function showMessage(message, error) {
            text = message
            isError = error
            color = error ? "#f44336" : "#4caf50"
            border.color = error ? "#d32f2f" : "#388e3c"
            visible = true
            hideStatusTimer.restart()
        }
        
        Text {
            id: statusText
            anchors.centerIn: parent
            color: "white"
            font.bold: true
        }
        
        Timer {
            id: hideStatusTimer
            interval: 3000
            onTriggered: statusMessage.visible = false
        }
    }
} 