// A settings row with a label/subtext and a custom-drawn switch on the right.
import QtQuick
import QtQuick.Layouts

RowLayout {
    id: root

    signal toggled(bool on)

    property string text: ""
    property string subtext: ""
    property bool checked: false
    property var colors: null

    spacing: Style.spacingLarge
    Layout.fillWidth: true
    Layout.preferredHeight: Math.max(textColumn.implicitHeight, 30)

    ColumnLayout {
        id: textColumn
        Layout.fillWidth: true
        spacing: 1

        Text {
            Layout.fillWidth: true
            text: root.text
            font.family: Style.fontFamily
            font.pixelSize: Style.fontBodyLarge
            color: colors ? colors.surfaceText : "#e0e0e0"
            elide: Text.ElideRight
        }
        Text {
            Layout.fillWidth: true
            visible: root.subtext.length > 0
            text: root.subtext
            font.family: Style.fontFamily
            font.pixelSize: Style.fontCaption
            color: colors ? colors.surfaceVariantText : "#b0b0b0"
            wrapMode: Text.WordWrap
        }
    }

    Item {
        Layout.preferredWidth: 46
        Layout.preferredHeight: 26
        Layout.alignment: Qt.AlignVCenter

        Rectangle {
            id: track
            anchors.fill: parent
            radius: height / 2
            color: root.checked
                ? (colors ? colors.primary : Style.fallbackAccent)
                : (colors ? colors.surfaceVariant : "#666666")
            Behavior on color { ColorAnimation { duration: Style.animNormal } }
        }

        Rectangle {
            id: thumb
            width: 20
            height: 20
            radius: width / 2
            color: "#ffffff"
            x: root.checked ? track.width - width - 3 : 3
            y: (track.height - height) / 2
            Behavior on x { NumberAnimation { duration: Style.animNormal; easing.type: Easing.InOutQuad } }
        }

        MouseArea {
            id: clickArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: root.toggled(!root.checked)
        }
    }
}
