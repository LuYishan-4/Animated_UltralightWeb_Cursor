// Section heading used to group setting rows.
import QtQuick
import QtQuick.Layouts

RowLayout {
    id: root

    property string text: ""
    property var colors: null
    property bool first: false

    Layout.topMargin: first ? 0 : Style.spacingLarge
    Layout.fillWidth: true

    Text {
        Layout.fillWidth: true
        text: root.text
        font.family: Style.fontFamilyHeading
        font.pixelSize: Style.fontTitle
        font.weight: Font.DemiBold
        color: colors ? colors.surfaceText : "#e0e0e0"
        elide: Text.ElideRight
    }
}
