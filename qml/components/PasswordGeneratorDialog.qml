import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtSecretTool 1.0

Dialog {
    id: passwordGeneratorDialog
    title: qsTr("密码生成器")
    modal: true
    width: 400
    height: 350
    
    property string generatedPassword: ""
    signal passwordAccepted(string password)
    
    function generateNewPassword() {
        var length = lengthSlider.value
        var includeSymbols = symbolsCheckBox.checked
        generatedPassword = App.passwordManager.generatePassword(length, includeSymbols)
        passwordField.text = generatedPassword
    }
    
    onOpened: {
        generateNewPassword()
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 15
        
        // 密码长度设置
        GroupBox {
            Layout.fillWidth: true
            title: qsTr("密码长度")
            
            ColumnLayout {
                anchors.fill: parent
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: qsTr("长度:")
                        color: "#333"
                    }
                    
                    Slider {
                        id: lengthSlider
                        Layout.fillWidth: true
                        from: 4
                        to: 32
                        value: 12
                        stepSize: 1
                        
                        onValueChanged: generateNewPassword()
                    }
                    
                    Text {
                        text: Math.round(lengthSlider.value).toString()
                        color: "#333"
                        font.bold: true
                        width: 30
                    }
                }
            }
        }
        
        // 密码选项
        GroupBox {
            Layout.fillWidth: true
            title: qsTr("密码选项")
            
            ColumnLayout {
                anchors.fill: parent
                
                CheckBox {
                    id: symbolsCheckBox
                    text: qsTr("包含特殊字符 (!@#$%^&*)")
                    checked: true
                    onCheckedChanged: generateNewPassword()
                }
                
                CheckBox {
                    text: qsTr("包含数字 (0-9)")
                    checked: true
                    enabled: false  // 暂时不可配置，总是包含
                }
                
                CheckBox {
                    text: qsTr("包含小写字母 (a-z)")
                    checked: true
                    enabled: false  // 暂时不可配置，总是包含
                }
                
                CheckBox {
                    text: qsTr("包含大写字母 (A-Z)")
                    checked: true
                    enabled: false  // 暂时不可配置，总是包含
                }
            }
        }
        
        // 生成的密码
        GroupBox {
            Layout.fillWidth: true
            title: qsTr("生成的密码")
            
            ColumnLayout {
                anchors.fill: parent
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    TextField {
                        id: passwordField
                        Layout.fillWidth: true
                        readOnly: true
                        selectByMouse: true
                        text: generatedPassword
                        font.family: "Courier, monospace"
                    }
                    
                    Button {
                        text: qsTr("复制")
                        onClicked: {
                            passwordField.selectAll()
                            passwordField.copy()
                            // TODO: 显示复制成功提示
                        }
                    }
                }
                
                Button {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("重新生成")
                    onClicked: generateNewPassword()
                }
            }
        }
        
        // 按钮区域
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 10
            
            Button {
                text: qsTr("取消")
                onClicked: passwordGeneratorDialog.close()
            }
            
            Button {
                text: qsTr("使用此密码")
                highlighted: true
                enabled: generatedPassword.length > 0
                onClicked: {
                    passwordAccepted(generatedPassword)
                    passwordGeneratorDialog.close()
                }
            }
        }
    }
} 