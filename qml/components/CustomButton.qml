import QtQuick 2.15
import QtQuick.Controls 2.15

/**
 * @brief 自定义按钮组件
 */
Button {
    id: customButton
    
    property color normalColor: "#f0f0f0"
    property color hoverColor: "#e0e0e0"
    property color pressedColor: "#d0d0d0"
    property color textColor: "#333"
    property int borderRadius: 4
    property bool isTransparent: false
    
    height: 36
    
    background: Rectangle {
        color: {
            if (isTransparent) return "transparent"
            if (customButton.pressed) return pressedColor
            if (customButton.hovered) return hoverColor
            return normalColor
        }
        border.color: isTransparent ? "transparent" : "#ccc"
        border.width: 1
        radius: borderRadius
        
        Behavior on color {
            ColorAnimation { duration: 100 }
        }
    }
    
    contentItem: Text {
        text: customButton.text
        font: customButton.font
        color: textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    
    // 点击效果
    scale: pressed ? 0.98 : 1.0
    Behavior on scale {
        NumberAnimation { duration: 50 }
    }
} 