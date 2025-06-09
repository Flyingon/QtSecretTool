import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtSecretTool 1.0

Rectangle {
    id: addEditPasswordPage
    color: "white"
    
    property bool isEditMode: false
    property var currentPasswordItem: null
    
    signal passwordSaved()
    signal cancelled()
    
    function openForAdd() {
        isEditMode = false
        currentPasswordItem = null
        clearForm()
        console.log("Opening add password page")
    }
    
    function openForEdit(passwordItem) {
        isEditMode = true
        currentPasswordItem = passwordItem
        populateForm(passwordItem)
        console.log("Opening edit password page for:", passwordItem.title)
    }
    
    function clearForm() {
        titleField.text = ""
        usernameField.text = ""
        passwordField.text = ""
        websiteField.text = ""
        notesField.text = ""
        categoryComboBox.currentIndex = 0
        favoriteCheckBox.checked = false
    }
    
    function populateForm(item) {
        titleField.text = item.title || ""
        usernameField.text = item.username || ""
        passwordField.text = item.password || ""
        websiteField.text = item.website || ""
        notesField.text = item.notes || ""
        
        // 设置分类
        var categories = App.passwordManager.getCategories()
        var categoryIndex = categories.indexOf(item.category)
        categoryComboBox.currentIndex = categoryIndex >= 0 ? categoryIndex : 0
        
        favoriteCheckBox.checked = item.isFavorite || false
    }
    
    function savePassword() {
        // 验证必填字段
        if (titleField.text.trim() === "") {
            errorText.text = qsTr("标题不能为空")
            errorText.visible = true
            return
        }
        
        if (passwordField.text.trim() === "") {
            errorText.text = qsTr("密码不能为空")
            errorText.visible = true
            return
        }
        
        var success = false
        
        if (isEditMode && currentPasswordItem) {
            // 更新现有密码
            currentPasswordItem.title = titleField.text.trim()
            currentPasswordItem.username = usernameField.text.trim()
            currentPasswordItem.password = passwordField.text.trim()
            currentPasswordItem.website = websiteField.text.trim()
            currentPasswordItem.notes = notesField.text.trim()
            currentPasswordItem.category = categoryComboBox.currentText
            currentPasswordItem.isFavorite = favoriteCheckBox.checked
            
            success = App.passwordManager.updatePassword(currentPasswordItem)
        } else {
            // 创建新密码
            var newItem = App.passwordManager.createPasswordItem(
                titleField.text.trim(),
                usernameField.text.trim(),
                passwordField.text.trim(),
                websiteField.text.trim(),
                notesField.text.trim(),
                categoryComboBox.currentText
            )
            
            if (newItem) {
                newItem.isFavorite = favoriteCheckBox.checked
                success = App.passwordManager.savePassword(newItem)
            }
        }
        
        if (success) {
            passwordSaved()
            clearForm()
            errorText.visible = false
        } else {
            errorText.text = qsTr("保存密码失败: ") + App.passwordManager.lastError
            errorText.visible = true
        }
    }
    
    ScrollView {
        anchors.fill: parent
        anchors.margins: 20
        
        ColumnLayout {
            width: Math.min(600, addEditPasswordPage.width - 40)
            spacing: 20
            
            // 标题
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: isEditMode ? qsTr("编辑密码") : qsTr("添加新密码")
                font.pointSize: 20
                font.bold: true
                color: "#333"
            }
            
            // 错误消息
            Rectangle {
                id: errorText
                visible: false
                Layout.fillWidth: true
                height: 40
                color: "#ffebee"
                border.color: "#f44336"
                border.width: 1
                radius: 4
                
                property alias text: errorLabel.text
                
                Text {
                    id: errorLabel
                    anchors.centerIn: parent
                    color: "#d32f2f"
                    font.pointSize: 10
                }
            }
            
            // 表单
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("基本信息")
                
                GridLayout {
                    anchors.fill: parent
                    columns: 2
                    columnSpacing: 10
                    rowSpacing: 15
                    
                    // 标题
                    Label {
                        text: qsTr("标题 *")
                        color: "#333"
                    }
                    
                    TextField {
                        id: titleField
                        Layout.fillWidth: true
                        placeholderText: qsTr("例如: Gmail账户")
                        selectByMouse: true
                    }
                    
                    // 用户名
                    Label {
                        text: qsTr("用户名")
                        color: "#333"
                    }
                    
                    TextField {
                        id: usernameField
                        Layout.fillWidth: true
                        placeholderText: qsTr("例如: user@example.com")
                        selectByMouse: true
                    }
                    
                    // 密码
                    Label {
                        text: qsTr("密码 *")
                        color: "#333"
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        
                        TextField {
                            id: passwordField
                            Layout.fillWidth: true
                            placeholderText: qsTr("输入密码")
                            echoMode: showPasswordButton.checked ? TextInput.Normal : TextInput.Password
                            selectByMouse: true
                        }
                        
                        Button {
                            id: showPasswordButton
                            checkable: true
                            text: checked ? qsTr("隐藏") : qsTr("显示")
                            implicitWidth: 60
                        }
                        
                        Button {
                            text: qsTr("生成")
                            implicitWidth: 60
                            onClicked: {
                                passwordField.text = App.passwordManager.generatePassword(12, true)
                            }
                        }
                    }
                    
                    // 网站
                    Label {
                        text: qsTr("网站")
                        color: "#333"
                    }
                    
                    TextField {
                        id: websiteField
                        Layout.fillWidth: true
                        placeholderText: qsTr("例如: https://www.example.com")
                        selectByMouse: true
                    }
                    
                    // 分类
                    Label {
                        text: qsTr("分类")
                        color: "#333"
                    }
                    
                    ComboBox {
                        id: categoryComboBox
                        Layout.fillWidth: true
                        editable: true
                        model: ["工作", "个人", "社交", "金融", "娱乐", "其他"]
                    }
                    
                    // 收藏
                    Label {
                        text: qsTr("收藏")
                        color: "#333"
                    }
                    
                    CheckBox {
                        id: favoriteCheckBox
                        text: qsTr("加入收藏夹")
                    }
                }
            }
            
            // 备注
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("备注")
                
                ScrollView {
                    anchors.fill: parent
                    implicitHeight: 120
                    
                    TextArea {
                        id: notesField
                        placeholderText: qsTr("可选的备注信息...")
                        selectByMouse: true
                        wrapMode: TextArea.Wrap
                    }
                }
            }
            
            // 按钮
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 10
                
                Button {
                    text: qsTr("取消")
                    implicitWidth: 100
                    onClicked: {
                        clearForm()
                        cancelled()
                    }
                }
                
                Button {
                    text: isEditMode ? qsTr("更新") : qsTr("保存")
                    implicitWidth: 100
                    highlighted: true
                    onClicked: savePassword()
                }
            }
            
            // 底部间距
            Item {
                Layout.fillHeight: true
                Layout.minimumHeight: 20
            }
        }
    }
} 