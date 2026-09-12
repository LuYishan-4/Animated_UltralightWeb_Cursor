// Compact icon button. Mirrors the reference plugin's IconButton API (an
// `icon` name plus a `tonal` style) but renders from a small built-in glyph
// table instead of the "Material Design Icons" font, which is not guaranteed
// to be installed on a standalone desktop.
import QtQuick

Rectangle {
    id: root

    signal clicked()

    property string icon: ""
    property var colors: null
    property bool tonal: false

    readonly property string glyph: {
        switch (icon) {
        case "close": return "\u2715"       // ✕
        case "delete": return "\u2715"      // ✕
        case "add": return "\uFF0B"        // ＋
        case "refresh": return "\u21BB"    // ↻
        case "check": return "\u2713"      // ✓
        case "play_arrow": return "\u25B6" // ▶
        case "folder_open": return "\u25A3" // ▣
        default: return icon.length > 0 ? icon : ""
        }
    }

    implicitWidth: 30
    implicitHeight: 30
    radius: Style.radiusMedium
    color: root.tonal
        ? (colors ? colors.surfaceVariant : "#33ffffff")
        : (mouse.containsMouse ? (colors ? colors.surfaceVariant : "#33ffffff") : "transparent")
    Behavior on color { ColorAnimation { duration: Style.animFast } }

    Text {
        anchors.centerIn: parent
        text: root.glyph
        font.family: Style.fontFamily
        font.pixelSize: 15
        color: colors ? colors.surfaceText : "#e0e0e0"
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
