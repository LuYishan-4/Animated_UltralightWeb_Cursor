import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    id: root

    property var backend

    property color bgColor: "#0d0d14"
    property color cardColor: "#1e1e2e"
    property color cardHoverColor: "#282838"
    property color accentColor: "#89b4fa"
    property color textColor: "#cdd6f4"
    property color subTextColor: "#7f849c"
    property color dangerColor: "#f38ba8"

    Rectangle {
        anchors.fill: parent
        color: bgColor
    }

    FolderDialog {
        id: themeUploadDialog
        title: qsTr("Choose a cursor theme folder")
        onAccepted: root.backend.uploadTheme(
            typeof selectedFolder === "string"
                ? selectedFolder
                : selectedFolder.toString().replace(/^file:\/\//, ""))
    }

    ScrollView {
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: Math.min(760, root.width - 48)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 32
            spacing: 10

            Label {
                text: qsTr("Web Cursor")
                font.pixelSize: 28
                font.bold: true
                color: textColor
                Layout.bottomMargin: 8
            }

            // ---- Enable / disable ----
            ToggleRow {
                Layout.fillWidth: true
                title: qsTr("Enable Web Cursor")
                subtext: qsTr("Render the selected HTML/CSS cursor across the desktop")
                checked: root.backend ? root.backend.enabled : false
                onToggled: function(on) {
                    if (on)
                        root.backend.enable();
                    else
                        root.backend.disable();
                }
            }

            SectionHeader {
                Layout.fillWidth: true
                Layout.topMargin: 10
                text: qsTr("Cursor Theme")
                actionText: qsTr("Import")
                onAction: themeUploadDialog.open()
            }

            // Current theme + reload
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 64
                radius: 12
                color: cardColor

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Label {
                            text: qsTr("Current theme")
                            font.pixelSize: 11
                            color: subTextColor
                        }
                        Label {
                            text: root.backend ? root.backend.currentTheme : ""
                            font.pixelSize: 15
                            font.bold: true
                            color: textColor
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }

                    Button {
                        text: qsTr("Reload")
                        onClicked: root.backend.reload()
                    }
                }
            }

            Repeater {
                model: root.backend ? root.backend.themeList : []

                delegate: Rectangle {
                    required property string modelData
                    readonly property var details: root.backend.getThemeDetails(modelData)
                    readonly property bool isCurrent: root.backend.currentTheme === modelData

                    Layout.fillWidth: true
                    implicitHeight: 76
                    radius: 12
                    color: cardColor
                    border.color: isCurrent ? accentColor : "transparent"
                    border.width: isCurrent ? 2 : 0

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 14

                        Image {
                            Layout.preferredWidth: 48
                            Layout.preferredHeight: 48
                            Layout.alignment: Qt.AlignVCenter
                            source: details.iconPath || ""
                            sourceSize.width: 96
                            sourceSize.height: 96
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            cache: false
                            visible: status === Image.Ready
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Label {
                                text: modelData
                                font.pixelSize: 14
                                font.bold: true
                                color: textColor
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                            Label {
                                text: details.describe || qsTr("No description provided")
                                font.pixelSize: 12
                                color: subTextColor
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                            Label {
                                text: qsTr("By %1 · minimum %2 × %3")
                                    .arg(details.author)
                                    .arg(details.minWidth)
                                    .arg(details.minHeight)
                                font.pixelSize: 11
                                color: subTextColor
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                        }

                        Button {
                            text: isCurrent ? qsTr("In Use") : qsTr("Apply")
                            enabled: !isCurrent
                            onClicked: root.backend.useTheme(modelData)
                        }
                        Button {
                            text: qsTr("Folder")
                            onClicked: root.backend.openThemeFolder(modelData)
                        }
                        Button {
                            text: qsTr("Remove")
                            onClicked: removeConfirmDialog.openFor(modelData)
                        }
                    }
                }
            }

            SectionHeader {
                Layout.fillWidth: true
                Layout.topMargin: 10
                text: qsTr("Size")
            }

            StepperRow {
                Layout.fillWidth: true
                title: qsTr("Cursor width")
                subtext: qsTr("Render width in pixels")
                minimum: 1
                maximum: 1920
                value: root.backend ? root.backend.cursorWidth : 128
                onChanged: function(v) { root.backend.cursorWidth = v; }
            }

            StepperRow {
                Layout.fillWidth: true
                title: qsTr("Cursor height")
                subtext: qsTr("Render height in pixels")
                minimum: 1
                maximum: 1080
                value: root.backend ? root.backend.cursorHeight : 128
                onChanged: function(v) { root.backend.cursorHeight = v; }
            }

            SectionHeader {
                Layout.fillWidth: true
                Layout.topMargin: 10
                text: qsTr("Rendering")
            }

            ToggleRow {
                Layout.fillWidth: true
                title: qsTr("GPU rendering")
                subtext: qsTr("Use hardware-accelerated compositing when available")
                checked: root.backend ? root.backend.gpuRender : true
                onToggled: function(on) { root.backend.gpuRender = on; }
            }

            ToggleRow {
                Layout.fillWidth: true
                title: qsTr("Launch on startup")
                subtext: qsTr("Automatically start the cursor engine after login")
                checked: root.backend ? root.backend.autostart : true
                onToggled: function(on) { root.backend.setAutostart(on); }
            }

            SectionHeader {
                Layout.fillWidth: true
                Layout.topMargin: 10
                text: qsTr("Ignored Applications")
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: blacklistColumn.implicitHeight + 24
                radius: 12
                color: cardColor

                ColumnLayout {
                    id: blacklistColumn
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 8

                    Repeater {
                        model: root.backend ? root.backend.blacklist : []

                        delegate: RowLayout {
                            required property string modelData
                            Layout.fillWidth: true

                            Label {
                                text: modelData
                                color: textColor
                                font.pixelSize: 13
                                Layout.fillWidth: true
                                elide: Text.ElideMiddle
                            }
                            Button {
                                text: qsTr("Remove")
                                onClicked: root.backend.removeBlacklist(modelData)
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        TextField {
                            id: blacklistInput
                            Layout.fillWidth: true
                            placeholderText: qsTr("Window class or application name")
                            onAccepted: addBlacklistEntry()
                        }
                        Button {
                            text: qsTr("Add")
                            enabled: blacklistInput.text.trim().length > 0
                            onClicked: addBlacklistEntry()
                        }
                    }

                    function addBlacklistEntry() {
                        const value = blacklistInput.text.trim();
                        if (value.length === 0)
                            return;
                        root.backend.addBlacklist(value);
                        blacklistInput.text = "";
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                visible: root.backend && root.backend.statusMessage.length > 0
                text: root.backend ? root.backend.statusMessage : ""
                color: subTextColor
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }

            // Bottom spacing
            Item { Layout.preferredHeight: 32 }
        }
    }

    Dialog {
        id: removeConfirmDialog
        title: qsTr("Remove Theme")
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        anchors.centerIn: parent

        property string themeName: ""

        function openFor(name) {
            themeName = name;
            open();
        }

        Label {
            text: qsTr("Remove theme \"%1\"? This cannot be undone.").arg(removeConfirmDialog.themeName)
            wrapMode: Text.WordWrap
        }

        onAccepted: root.backend.removeTheme(removeConfirmDialog.themeName)
    }

    // ---------- reusable components ----------

    component SectionHeader: RowLayout {
        id: sectionHeader
        property string text: ""
        property string actionText: ""
        signal action()

        Layout.fillWidth: true
        Layout.topMargin: 8
        spacing: 12

        Label {
            text: sectionHeader.text
            font.pixelSize: 18
            font.bold: true
            color: root.textColor
            Layout.fillWidth: true
        }

        Button {
            visible: actionText.length > 0
            text: sectionHeader.actionText
            onClicked: sectionHeader.action()
        }
    }

    component ToggleRow: Rectangle {
        id: toggleRow
        property string title: ""
        property string subtext: ""
        property bool checked: false
        signal toggled(bool on)

        Layout.fillWidth: true
        implicitHeight: rowLayout.implicitHeight + 28
        radius: 12
        color: root.cardColor

        RowLayout {
            id: rowLayout
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    text: toggleRow.title
                    font.pixelSize: 14
                    color: root.textColor
                }
                Label {
                    text: toggleRow.subtext
                    font.pixelSize: 12
                    color: root.subTextColor
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }

            Switch {
                id: switchControl
                checked: toggleRow.checked
                onToggled: toggleRow.toggled(checked)
            }
        }
    }

    component StepperRow: Rectangle {
        id: stepperRow
        property string title: ""
        property string subtext: ""
        property int minimum: 1
        property int maximum: 1920
        property alias value: spinBox.value
        signal changed(int value)

        Layout.fillWidth: true
        implicitHeight: rowLayout.implicitHeight + 28
        radius: 12
        color: root.cardColor

        RowLayout {
            id: rowLayout
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    text: stepperRow.title
                    font.pixelSize: 14
                    color: root.textColor
                }
                Label {
                    text: stepperRow.subtext
                    font.pixelSize: 12
                    color: root.subTextColor
                    Layout.fillWidth: true
                }
            }

            SpinBox {
                id: spinBox
                from: stepperRow.minimum
                to: stepperRow.maximum
                editable: true
                onValueModified: stepperRow.changed(spinBox.value)
            }
        }
    }
}
