import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import QtSecretTool 1.0

Dialog {
    id: masterPasswordDialog
    title: qsTr("主密码设置")
    modal: true
    width: 400
    height: 300
    anchors.centerIn: parent
    
    property bool isFirstTime: false
    property bool isChangingPassword: false
    
    signal passwordSet()
    signal passwordChanged()
    
    // 密码管理器
    PasswordManager {
        id: passwordManager
        onMasterPasswordSet: {
            console.log("Master password set successfully")
            masterPasswordDialog.accept()
            passwordSet()
        }
        onMasterPasswordVerified: {
            console.log("Master password verified successfully")
            masterPasswordDialog.accept()
        }
        onMasterPasswordChanged: {
            console.log("Master password changed successfully")
            masterPasswordDialog.accept()
            passwordChanged()
        }
        onPasswordError: function(error) {
            console.log("Password error:", error)
            showError(error)
        }
    }
    
    onOpened: {
        if (isFirstTime) {
            title = qsTr("设置主密码")
            subtitleText.text = qsTr("请设置一个主密码来保护您的密码数据")
        } else if (isChangingPassword) {
            title = qsTr("更改主密码")
            subtitleText.text = qsTr("请输入旧密码和新密码")
        } else {
            title = qsTr("验证主密码")
            subtitleText.text = qsTr("请输入主密码以继续")
        }
    }
    
    background: Rectangle {
        color: "white"
        border.color: "#ddd"
        border.width: 1
        radius: 8
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20
        
        // 说明文字
        Text {
            id: subtitleText
            Layout.fillWidth: true
            text: qsTr("请输入主密码")
            color: "#666"
            font.pointSize: 12
            wrapMode: Text.WordWrap
        }
        
        // 旧密码输入（仅在更改密码时显示）
        ColumnLayout {
            visible: isChangingPassword
            Layout.fillWidth: true
            spacing: 8
            
            Text {
                text: qsTr("旧密码:")
                color: "#333"
                font.pointSize: 11
            }
            
            TextField {
                id: oldPasswordField
                Layout.fillWidth: true
                echoMode: TextInput.Password
                placeholderText: qsTr("请输入旧密码")
                background: Rectangle {
                    border.color: parent.focus ? "#2196F3" : "#ddd"
                    border.width: 1
                    radius: 4
                }
            }
        }
        
        // 新密码输入
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8
            
            Text {
                text: isChangingPassword ? qsTr("新密码:") : qsTr("密码:")
                color: "#333"
                font.pointSize: 11
            }
            
            TextField {
                id: newPasswordField
                Layout.fillWidth: true
                echoMode: TextInput.Password
                placeholderText: isChangingPassword ? qsTr("请输入新密码") : qsTr("请输入密码")
                background: Rectangle {
                    border.color: parent.focus ? "#2196F3" : "#ddd"
                    border.width: 1
                    radius: 4
                }
            }
        }
        
        // 确认密码输入
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8
            
            Text {
                text: isChangingPassword ? qsTr("确认新密码:") : qsTr("确认密码:")
                color: "#333"
                font.pointSize: 11
            }
            
            TextField {
                id: confirmPasswordField
                Layout.fillWidth: true
                echoMode: TextInput.Password
                placeholderText: isChangingPassword ? qsTr("请再次输入新密码") : qsTr("请再次输入密码")
                background: Rectangle {
                    border.color: parent.focus ? "#2196F3" : "#ddd"
                    border.width: 1
                    radius: 4
                }
            }
        }
        
        // 密码强度指示器
        Rectangle {
            Layout.fillWidth: true
            height: 4
            radius: 2
            color: "#e0e0e0"
            
            Rectangle {
                width: parent.width * passwordStrength
                height: parent.height
                radius: parent.radius
                color: passwordStrengthColor
            }
        }
        
        Text {
            text: passwordStrengthText
            color: passwordStrengthColor
            font.pointSize: 10
        }
        
        // 按钮
        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight
            spacing: 10
            
            Button {
                text: qsTr("取消")
                onClicked: masterPasswordDialog.reject()
                background: Rectangle {
                    color: parent.hovered ? "#f5f5f5" : "transparent"
                    border.color: "#ddd"
                    border.width: 1
                    radius: 4
                }
            }
            
            Button {
                text: isFirstTime ? qsTr("设置") : (isChangingPassword ? qsTr("更改") : qsTr("确定"))
                enabled: canProceed
                onClicked: handlePasswordAction()
                background: Rectangle {
                    color: parent.enabled ? (parent.hovered ? "#1976D2" : "#2196F3") : "#e0e0e0"
                    border.color: parent.enabled ? "#1976D2" : "#bdbdbd"
                    border.width: 1
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.enabled ? "white" : "#9e9e9e"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
    
    // 计算属性
    property real passwordStrength: {
        let password = newPasswordField.text
        if (password.length === 0) return 0
        
        let strength = 0
        if (password.length >= 8) strength += 0.2
        if (/[a-z]/.test(password)) strength += 0.2
        if (/[A-Z]/.test(password)) strength += 0.2
        if (/[0-9]/.test(password)) strength += 0.2
        if (/[^a-zA-Z0-9]/.test(password)) strength += 0.2
        
        return Math.min(strength, 1.0)
    }
    
    property string passwordStrengthColor: {
        if (passwordStrength < 0.4) return "#f44336"
        if (passwordStrength < 0.7) return "#ff9800"
        return "#4caf50"
    }
    
    property string passwordStrengthText: {
        if (passwordStrength < 0.4) return qsTr("弱")
        if (passwordStrength < 0.7) return qsTr("中等")
        return qsTr("强")
    }
    
    property bool canProceed: {
        if (isChangingPassword) {
            return oldPasswordField.text.length > 0 && 
                   newPasswordField.text.length > 0 && 
                   confirmPasswordField.text.length > 0 &&
                   newPasswordField.text === confirmPasswordField.text
        } else {
            return newPasswordField.text.length > 0 && 
                   confirmPasswordField.text.length > 0 &&
                   newPasswordField.text === confirmPasswordField.text
        }
    }
    
    // 处理密码操作
    function handlePasswordAction() {
        if (isChangingPassword) {
            // 更改密码
            if (oldPasswordField.text.length === 0) {
                showError(qsTr("请输入旧密码"))
                return
            }
            
            if (newPasswordField.text !== confirmPasswordField.text) {
                showError(qsTr("新密码和确认密码不匹配"))
                return
            }
            
            // 调用C++方法更改密码
            passwordManager.changeMasterPassword(oldPasswordField.text, newPasswordField.text)
            
        } else if (isFirstTime) {
            // 首次设置密码
            if (newPasswordField.text !== confirmPasswordField.text) {
                showError(qsTr("密码和确认密码不匹配"))
                return
            }
            
            // 调用C++方法设置密码
            passwordManager.setMasterPassword(newPasswordField.text)
            
        } else {
            // 验证密码
            // 调用C++方法验证密码
            passwordManager.verifyMasterPassword(newPasswordField.text)
        }
    }
    
    function showError(message) {
        // TODO: 显示错误消息
        console.log("Error:", message)
    }
    
    // 重置对话框
    function reset() {
        oldPasswordField.text = ""
        newPasswordField.text = ""
        confirmPasswordField.text = ""
    }
} 