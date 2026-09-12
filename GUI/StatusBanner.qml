import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Frame {
    id: root

    required property string message
    property string level: "info"

    visible: message.length > 0
    Layout.fillWidth: true
    padding: 14

    readonly property color accent: {
        if (level === "error") return "#ef5350"
        if (level === "success") return "#4caf50"
        if (level === "warning") return "#ffb300"
        return Material.accent
    }

    background: Rectangle {
        radius: 12
        color: Qt.rgba(root.accent.r, root.accent.g, root.accent.b, 0.12)
        border.color: Qt.rgba(root.accent.r, root.accent.g, root.accent.b, 0.5)
        border.width: 1
    }

    RowLayout {
        width: parent.width
        spacing: 12

        Rectangle {
            Layout.preferredWidth: 8
            Layout.preferredHeight: 8
            radius: 4
            color: root.accent
        }

        Label {
            Layout.fillWidth: true
            text: root.message
            wrapMode: Text.WordWrap
            color: palette.text
        }
    }
}
