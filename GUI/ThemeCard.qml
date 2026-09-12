pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Frame {
    id: root

    required property string themeName
    required property var details
    required property bool current

    signal applyRequested()
    signal openRequested()
    signal removeRequested()

    Layout.fillWidth: true
    implicitHeight: 144
    padding: 14

    background: Rectangle {
        radius: 14
        color: root.current
            ? Qt.rgba(Material.accent.r, Material.accent.g, Material.accent.b, 0.10)
            : palette.base
        border.color: root.current ? Material.accent : palette.mid
        border.width: root.current ? 2 : 1
    }

    RowLayout {
        anchors.fill: parent
        spacing: 14

        Rectangle {
            Layout.preferredWidth: 84
            Layout.preferredHeight: 84
            Layout.alignment: Qt.AlignVCenter
            radius: 12
            color: palette.alternateBase
            clip: true

            Label {
                anchors.centerIn: parent
                visible: preview.status !== Image.Ready
                text: root.themeName.length > 0
                    ? root.themeName.charAt(0).toUpperCase()
                    : "?"
                font.pixelSize: 28
                font.bold: true
                color: palette.mid
            }

            Image {
                id: preview
                anchors.fill: parent
                anchors.margins: 8
                source: root.details.iconPath || ""
                fillMode: Image.PreserveAspectFit
                asynchronous: true
                cache: false
                smooth: true
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4

            RowLayout {
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true
                    text: root.details.displayName || root.themeName
                    font.pixelSize: 16
                    font.bold: true
                    elide: Text.ElideRight
                }

                Label {
                    visible: root.current
                    text: qsTr("Active")
                    font.pixelSize: 11
                    font.bold: true
                    color: Material.accent
                }

                Label {
                    visible: root.details.builtIn || false
                    text: qsTr("Built in")
                    font.pixelSize: 11
                    color: palette.placeholderText
                }
            }

            Label {
                Layout.fillWidth: true
                text: root.details.description || qsTr("HTML cursor theme")
                color: palette.placeholderText
                elide: Text.ElideRight
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("By %1 · %2 × %3 minimum")
                    .arg(root.details.author || qsTr("Unknown"))
                    .arg(root.details.minWidth || 128)
                    .arg(root.details.minHeight || 128)
                font.pixelSize: 11
                color: palette.placeholderText
                elide: Text.ElideRight
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Button {
                    text: root.current ? qsTr("In use") : qsTr("Use theme")
                    enabled: !root.current
                    highlighted: !root.current
                    Accessible.name: text
                    onClicked: root.applyRequested()
                }

                Item { Layout.fillWidth: true }

                ToolButton {
                    text: "↗"
                    Accessible.name: qsTr("Open theme folder")
                    ToolTip.visible: hovered
                    ToolTip.text: Accessible.name
                    onClicked: root.openRequested()
                }

                ToolButton {
                    visible: !(root.details.builtIn || false) && !root.current
                    text: "×"
                    Accessible.name: qsTr("Remove imported theme")
                    ToolTip.visible: hovered
                    ToolTip.text: Accessible.name
                    onClicked: root.removeRequested()
                }
            }
        }
    }
}
