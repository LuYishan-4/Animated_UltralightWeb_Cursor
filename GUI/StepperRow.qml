// Settings row with a label/subtext and an integer stepper on the right.
import QtQuick
import QtQuick.Layouts

RowLayout {
    id: root

    signal moved(int value)

    property string text: ""
    property string subtext: ""
    property int min: 1
    property int max: 100
    property int value: 0
    property var colors: null

    spacing: Style.spacingLarge
    Layout.fillWidth: true
    Layout.preferredHeight: Math.max(textColumn.implicitHeight, 34)

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

    RowLayout {
        Layout.alignment: Qt.AlignVCenter
        spacing: Style.spacingSmall

        StepButton {
            glyph: "−"
            colors: root.colors
            clickEnabled: root.value > root.min
            onClicked: root.moved(root.value - 1)
        }

        Text {
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: root.value
            font.family: Style.fontFamilyMono
            font.pixelSize: Style.fontBodyLarge
            color: colors ? colors.surfaceText : "#e0e0e0"
            Layout.minimumWidth: 28
        }

        StepButton {
            glyph: "+"
            colors: root.colors
            clickEnabled: root.value < root.max
            onClicked: root.moved(root.value + 1)
        }
    }

    component StepButton : Rectangle {
        signal clicked()
        property string glyph: ""
        property var colors: null
        property bool clickEnabled: true

        width: 28
        height: 28
        radius: 8
        opacity: clickEnabled ? 1 : 0.4
        color: mouse.containsMouse && clickEnabled
            ? (colors ? colors.surfaceVariant : "#33ffffff")
            : "transparent"
        Behavior on color { ColorAnimation { duration: Style.animFast } }

        Text {
            anchors.centerIn: parent
            text: parent.glyph
            font.family: Style.fontFamily
            font.pixelSize: 18
            baselineOffset: -1
            color: !parent.clickEnabled
                ? (parent.colors ? parent.colors.outline : "#808080")
                : (parent.colors ? parent.colors.surfaceText : "#e0e0e0")
        }

        MouseArea {
            id: mouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: { if (parent.clickEnabled) parent.clicked() }
        }
    }
}
