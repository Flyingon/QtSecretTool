import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * @brief 密码卡片组件
 */
Rectangle {
    id: passwordCard
    
    property var passwordItem: null
    
    signal clicked()
    signal editRequested()
    signal deleteRequested()
    signal favoriteToggled()
    
    height: 80
    color: mouseArea.containsMouse ? "#f8f9fa" : "white"
    border.color: "#e9ecef"
    border.width: 1
    radius: 6
    
    Behavior on color {
        ColorAnimation { duration: 150 }
    }
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: passwordCard.clicked()
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12
        
        // 左侧图标和主要信息
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            
            // 网站图标（使用文字作为占位符）
            Rectangle {
                width: 48
                height: 48
                radius: 24
                color: "#e3f2fd"
                border.color: "#2196f3"
                border.width: 1
                
                Text {
                    anchors.centerIn: parent
                    text: passwordItem && passwordItem.title ? 
                          passwordItem.title.substring(0, 1).toUpperCase() : "?"
                    font.pointSize: 18
                    font.bold: true
                    color: "#2196f3"
                }
            }
            
            // 主要信息
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: passwordItem ? passwordItem.title : ""
                        font.pointSize: 14
                        font.bold: true
                        color: "#212529"
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    
                    // 收藏星标
                    Text {
                        visible: passwordItem && passwordItem.isFavorite
                        text: "⭐"
                        font.pointSize: 12
                    }
                }
                
                Text {
                    text: passwordItem ? passwordItem.username : ""
                    font.pointSize: 12
                    color: "#6c757d"
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: passwordItem && passwordItem.website ? passwordItem.website : ""
                        font.pointSize: 10
                        color: "#adb5bd"
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    
                    Rectangle {
                        visible: passwordItem && passwordItem.category
                        height: 16
                        width: categoryText.width + 8
                        radius: 8
                        color: "#e7f3ff"
                        border.color: "#b3d9ff"
                        border.width: 1
                        
                        Text {
                            id: categoryText
                            anchors.centerIn: parent
                            text: passwordItem ? passwordItem.category : ""
                            font.pointSize: 8
                            color: "#0066cc"
                        }
                    }
                }
            }
        }
        
        // 右侧操作按钮
        RowLayout {
            spacing: 4
            
            // 复制密码按钮
            Button {
                width: 32
                height: 32
                background: Rectangle {
                    color: parent.hovered ? "#e9ecef" : "transparent"
                    radius: 4
                }
                
                Text {
                    anchors.centerIn: parent
                    text: "📋"
                    font.pointSize: 12
                }
                
                ToolTip.text: qsTr("复制密码")
                ToolTip.visible: hovered
                
                onClicked: {
                    if (passwordItem) {
                        // TODO: 复制密码到剪贴板
                        console.log("Copy password for:", passwordItem.title)
                    }
                }
            }
            
            // 收藏按钮
            Button {
                width: 32
                height: 32
                background: Rectangle {
                    color: parent.hovered ? "#e9ecef" : "transparent"
                    radius: 4
                }
                
                Text {
                    anchors.centerIn: parent
                    text: passwordItem && passwordItem.isFavorite ? "⭐" : "☆"
                    font.pointSize: 12
                    color: passwordItem && passwordItem.isFavorite ? "#ffc107" : "#6c757d"
                }
                
                ToolTip.text: passwordItem && passwordItem.isFavorite ? 
                              qsTr("取消收藏") : qsTr("添加收藏")
                ToolTip.visible: hovered
                
                onClicked: {
                    passwordCard.favoriteToggled()
                }
            }
            
            // 编辑按钮
            Button {
                width: 32
                height: 32
                background: Rectangle {
                    color: parent.hovered ? "#e9ecef" : "transparent"
                    radius: 4
                }
                
                Text {
                    anchors.centerIn: parent
                    text: "✏️"
                    font.pointSize: 12
                }
                
                ToolTip.text: qsTr("编辑")
                ToolTip.visible: hovered
                
                onClicked: {
                    passwordCard.editRequested()
                }
            }
            
            // 删除按钮
            Button {
                width: 32
                height: 32
                background: Rectangle {
                    color: parent.hovered ? "#ffebee" : "transparent"
                    radius: 4
                }
                
                Text {
                    anchors.centerIn: parent
                    text: "🗑️"
                    font.pointSize: 12
                }
                
                ToolTip.text: qsTr("删除")
                ToolTip.visible: hovered
                
                onClicked: {
                    passwordCard.deleteRequested()
                }
            }
        }
    }
} 