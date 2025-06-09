import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * @brief 搜索框组件
 */
Rectangle {
    id: searchBar
    
    property alias placeholderText: textField.placeholderText
    property alias searchText: textField.text
    property alias clearable: clearButton.visible
    
    signal textChanged(string text)
    
    height: 40
    color: "white"
    border.color: textField.activeFocus ? "#2196F3" : "#e0e0e0"
    border.width: 1
    radius: 4
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8
        
        // 搜索图标
        Text {
            text: "🔍"
            font.pointSize: 12
            color: "#666"
        }
        
        // 输入框
        TextField {
            id: textField
            Layout.fillWidth: true
            placeholderText: qsTr("搜索...")
            background: null
            selectByMouse: true
            
            onTextChanged: {
                searchBar.textChanged(text)
            }
            
            Keys.onReturnPressed: {
                searchBar.textChanged(text)
            }
        }
        
        // 清除按钮
        Button {
            id: clearButton
            visible: textField.text.length > 0
            width: 20
            height: 20
            background: null
            
            Text {
                anchors.centerIn: parent
                text: "✕"
                font.pointSize: 10
                color: "#666"
            }
            
            onClicked: {
                textField.clear()
                textField.forceActiveFocus()
            }
        }
    }
} 