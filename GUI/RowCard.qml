// Rounded surface container for a single setting row / group.
//
// Put exactly one child inside; it is inset by `padding`. The surrounding
// layout sizes this card, so set the card's `implicitHeight` from the child's
// own implicit height when you use it inside a ColumnLayout, e.g.:
//
//     RowCard {
//         RowLayout { id: row }
//         implicitHeight: row.implicitHeight + padding * 2
//     }
import QtQuick

Rectangle {
    id: root

    default property alias content: contentItem.data
    property var colors: null

    readonly property int padding: Style.paddingLarge

    radius: Style.radiusXLarge
    color: colors ? colors.surface : "#00000000"
    border.color: colors ? colors.outline : "transparent"
    border.width: Style.borderThin * 0.5

    Item {
        id: contentItem
        anchors.fill: parent
        anchors.margins: root.padding
    }
}
