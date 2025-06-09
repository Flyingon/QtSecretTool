import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtSecretTool 1.0
import "../components"

/**
 * @brief 密码列表页面
 */
Rectangle {
    id: passwordListPage
    color: "white"
    
    signal editPasswordRequested(var passwordItem)
    
    function openPasswordDetails(passwordItem) {
        // TODO: 打开密码详情页面
        console.log("Opening password details for:", passwordItem.title)
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        // 顶部工具栏
        RowLayout {
            Layout.fillWidth: true
            
            Text {
                text: qsTr("密码列表")
                font.pointSize: 16
                font.bold: true
                color: "#333"
            }
            
            Item { Layout.fillWidth: true }
            
            // 排序选择
            ComboBox {
                id: sortComboBox
                model: [
                    qsTr("按更新时间"),
                    qsTr("按标题"),
                    qsTr("按分类"),
                    qsTr("按创建时间")
                ]
                currentIndex: 0
                
                onCurrentIndexChanged: {
                    switch(currentIndex) {
                    case 0:
                        App.passwordManager.passwordListModel.sortByUpdatedDate(false)
                        break
                    case 1:
                        App.passwordManager.passwordListModel.sortByTitle(true)
                        break
                    case 2:
                        App.passwordManager.passwordListModel.sortByCategory(true)
                        break
                    case 3:
                        App.passwordManager.passwordListModel.sortByCreatedDate(false)
                        break
                    }
                }
            }
            
            CustomButton {
                text: qsTr("添加密码")
                normalColor: "#4CAF50"
                hoverColor: "#45a049"
                pressedColor: "#3d8b40"
                textColor: "white"
                onClicked: {
                    // TODO: 切换到添加密码页面
                    console.log("Add password clicked")
                }
            }
        }
        
        // 密码列表
        ListView {
            id: passwordListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: App.passwordManager.passwordListModel
            clip: true
            spacing: 5
            
            // 空状态提示
            Label {
                anchors.centerIn: parent
                visible: passwordListView.count === 0
                text: qsTr("还没有保存任何密码\n点击'添加密码'开始使用")
                horizontalAlignment: Text.AlignHCenter
                color: "#999"
                font.pointSize: 14
            }
            
            delegate: PasswordCard {
                width: passwordListView.width
                passwordItem: model.passwordItem
                
                onClicked: {
                    passwordListPage.openPasswordDetails(passwordItem)
                }
                
                onEditRequested: {
                    passwordListPage.editPasswordRequested(passwordItem)
                }
                
                onDeleteRequested: {
                    deleteConfirmDialog.passwordToDelete = passwordItem
                    deleteConfirmDialog.open()
                }
                
                onFavoriteToggled: {
                    passwordItem.isFavorite = !passwordItem.isFavorite
                    App.passwordManager.updatePassword(passwordItem)
                }
            }
            
            // 滚动指示器
            ScrollIndicator.vertical: ScrollIndicator { }
        }
    }
    
    // 删除确认对话框
    Dialog {
        id: deleteConfirmDialog
        anchors.centerIn: parent
        width: 350
        height: 150
        title: qsTr("确认删除")
        modal: true
        
        property var passwordToDelete: null
        
        ColumnLayout {
            anchors.fill: parent
            
            Text {
                Layout.fillWidth: true
                text: qsTr("确定要删除密码 '%1' 吗？").arg(
                    deleteConfirmDialog.passwordToDelete ? 
                    deleteConfirmDialog.passwordToDelete.title : ""
                )
                wrapMode: Text.WordWrap
                color: "#333"
            }
            
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 20
                
                Item { Layout.fillWidth: true }
                
                CustomButton {
                    text: qsTr("取消")
                    onClicked: deleteConfirmDialog.close()
                }
                
                CustomButton {
                    text: qsTr("删除")
                    normalColor: "#f44336"
                    hoverColor: "#d32f2f"
                    pressedColor: "#b71c1c"
                    textColor: "white"
                    onClicked: {
                        if (deleteConfirmDialog.passwordToDelete) {
                            App.passwordManager.deletePassword(
                                deleteConfirmDialog.passwordToDelete.id
                            )
                        }
                        deleteConfirmDialog.close()
                    }
                }
            }
        }
    }
} 