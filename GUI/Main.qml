pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: window

    visible: true
    width: 980
    height: 760
    minimumWidth: 660
    minimumHeight: 540
    title: qsTr("Ultralight Web Cursor")

    Material.theme: Material.System
    Material.accent: Material.LightBlue

    required property var backend
    property string pendingRemoval: ""

    FolderDialog {
        id: importDialog
        title: qsTr("Import a cursor theme")
        onAccepted: window.backend.uploadTheme(selectedFolder)
    }

    Dialog {
        id: removeDialog
        anchors.centerIn: parent
        modal: true
        title: qsTr("Remove imported theme?")
        standardButtons: Dialog.Yes | Dialog.Cancel

        Label {
            width: 360
            text: qsTr("The theme “%1” will be permanently removed from your user data.")
                .arg(window.pendingRemoval)
            wrapMode: Text.WordWrap
        }

        onAccepted: {
            window.backend.removeTheme(window.pendingRemoval)
            window.pendingRemoval = ""
        }
        onRejected: window.pendingRemoval = ""
    }

    Dialog {
        id: uninstallDialog
        anchors.centerIn: parent
        modal: true
        title: qsTr("Uninstall Ultralight Web Cursor?")
        standardButtons: Dialog.Yes | Dialog.Cancel

        Label {
            width: 360
            text: qsTr("Windows will open the application uninstaller. Your user themes and settings are kept unless you remove them manually.")
            wrapMode: Text.WordWrap
        }

        onAccepted: window.backend.uninstall()
    }

    header: ToolBar {
        implicitHeight: 72

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 22
            anchors.rightMargin: 22
            spacing: 14

            Rectangle {
                Layout.preferredWidth: 44
                Layout.preferredHeight: 44
                radius: 13
                color: Qt.rgba(Material.accent.r, Material.accent.g,
                               Material.accent.b, 0.16)

                Image {
                    anchors.fill: parent
                    anchors.margins: 8
                    source: Qt.resolvedUrl("icons/io.github.luyishan4.ultralightwebcursor.svg")
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1

                Label {
                    text: qsTr("Ultralight Web Cursor")
                    font.pixelSize: 18
                    font.bold: true
                }
                Label {
                    text: window.backend.platformName
                    color: palette.placeholderText
                    font.pixelSize: 12
                }
            }

            Rectangle {
                Layout.preferredHeight: 32
                Layout.preferredWidth: engineStatus.implicitWidth + 26
                radius: 16
                color: window.backend.mainProcessConnected
                    ? Qt.rgba(0.30, 0.69, 0.31, 0.16)
                    : Qt.rgba(1.0, 0.70, 0.0, 0.16)

                Label {
                    id: engineStatus
                    anchors.centerIn: parent
                    text: window.backend.mainProcessConnected
                        ? qsTr("● Engine running")
                        : qsTr("○ Engine stopped")
                    color: window.backend.mainProcessConnected
                        ? "#66bb6a" : "#ffb300"
                    font.pixelSize: 12
                    font.bold: true
                }
            }

            Button {
                visible: !window.backend.mainProcessConnected
                text: qsTr("Start engine")
                highlighted: true
                onClicked: window.backend.startEngine()
            }
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: Math.min(900, window.width - 48)
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 16

            Item { Layout.preferredHeight: 10 }

            // Primary control -------------------------------------------------
            Frame {
                Layout.fillWidth: true
                padding: 0
                implicitHeight: 154

                background: Rectangle {
                    radius: 18
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop {
                            position: 0
                            color: Qt.rgba(Material.accent.r, Material.accent.g,
                                           Material.accent.b, 0.24)
                        }
                        GradientStop {
                            position: 1
                            color: Qt.rgba(0.55, 0.35, 0.85, 0.18)
                        }
                    }
                    border.color: Qt.rgba(Material.accent.r, Material.accent.g,
                                          Material.accent.b, 0.38)
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 24

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Label {
                            text: qsTr("Bring your cursor to life")
                            font.pixelSize: 24
                            font.bold: true
                        }
                        Label {
                            Layout.fillWidth: true
                            text: qsTr("Render programmable HTML, CSS and JavaScript themes as a desktop cursor.")
                            wrapMode: Text.WordWrap
                            color: palette.placeholderText
                        }
                        Label {
                            text: qsTr("Changes apply instantly while the engine is running.")
                            font.pixelSize: 12
                            color: Material.accent
                        }
                    }

                    ColumnLayout {
                        Layout.alignment: Qt.AlignVCenter
                        spacing: 6

                        Switch {
                            id: enableSwitch
                            Layout.alignment: Qt.AlignHCenter
                            checked: window.backend.enabled
                            Accessible.name: qsTr("Enable animated cursor")
                            onToggled: checked
                                ? window.backend.enable()
                                : window.backend.disable()
                        }
                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            text: enableSwitch.checked ? qsTr("Enabled") : qsTr("Disabled")
                            font.bold: true
                        }
                    }
                }
            }

            StatusBanner {
                message: window.backend.statusMessage
                level: window.backend.statusLevel
            }

            // Themes ----------------------------------------------------------
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 4

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    Label {
                        text: qsTr("Cursor themes")
                        font.pixelSize: 20
                        font.bold: true
                    }
                    Label {
                        text: qsTr("Choose a bundled theme or import your own trusted folder.")
                        color: palette.placeholderText
                    }
                }

                Button {
                    text: qsTr("Open theme folder")
                    onClicked: window.backend.openDataDirectory()
                }
                Button {
                    text: qsTr("Import theme…")
                    highlighted: true
                    onClicked: importDialog.open()
                }
            }

            Label {
                Layout.fillWidth: true
                visible: window.backend.themeList.length === 0
                text: qsTr("No valid themes were found. Reinstall the package or import a theme.")
                color: palette.placeholderText
                horizontalAlignment: Text.AlignHCenter
                padding: 28
            }

            GridLayout {
                id: themeGrid
                Layout.fillWidth: true
                columns: width >= 760 ? 2 : 1
                columnSpacing: 14
                rowSpacing: 14

                Repeater {
                    model: window.backend.themeList

                    ThemeCard {
                        id: themeCard
                        required property string modelData

                        Layout.columnSpan: 1
                        Layout.fillWidth: true
                        themeName: modelData
                        details: window.backend.getThemeDetails(modelData)
                        current: window.backend.currentTheme === modelData
                        onApplyRequested: window.backend.useTheme(themeName)
                        onOpenRequested: window.backend.openThemeFolder(themeName)
                        onRemoveRequested: {
                            window.pendingRemoval = themeName
                            removeDialog.open()
                        }
                    }
                }
            }

            // Preferences -----------------------------------------------------
            Label {
                Layout.topMargin: 8
                text: qsTr("Preferences")
                font.pixelSize: 20
                font.bold: true
            }

            GridLayout {
                Layout.fillWidth: true
                columns: width >= 760 ? 2 : 1
                columnSpacing: 14
                rowSpacing: 14

                Frame {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 172
                    padding: 18

                    background: Rectangle {
                        radius: 14
                        color: palette.base
                        border.color: palette.mid
                        border.width: 1
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 10

                        Label {
                            text: qsTr("Cursor size")
                            font.pixelSize: 16
                            font.bold: true
                        }
                        Label {
                            text: qsTr("Set an exact render size. Theme minimums are shown on each card.")
                            color: palette.placeholderText
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                        Item { Layout.fillHeight: true }
                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: qsTr("Width") }
                            SpinBox {
                                Layout.fillWidth: true
                                from: 16
                                to: 4096
                                stepSize: 8
                                editable: true
                                value: window.backend.cursorWidth
                                onValueModified: window.backend.cursorWidth = value
                            }
                            Label { text: "×" }
                            Label { text: qsTr("Height") }
                            SpinBox {
                                Layout.fillWidth: true
                                from: 16
                                to: 4096
                                stepSize: 8
                                editable: true
                                value: window.backend.cursorHeight
                                onValueModified: window.backend.cursorHeight = value
                            }
                        }
                    }
                }

                Frame {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 172
                    padding: 18

                    background: Rectangle {
                        radius: 14
                        color: palette.base
                        border.color: palette.mid
                        border.width: 1
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 10

                        Label {
                            text: qsTr("Launch on login")
                            font.pixelSize: 16
                            font.bold: true
                        }
                        Label {
                            Layout.fillWidth: true
                            text: qsTr("Start the background cursor engine automatically for this user.")
                            color: palette.placeholderText
                            wrapMode: Text.WordWrap
                        }
                        Item { Layout.fillHeight: true }
                        Switch {
                            text: checked ? qsTr("Starts automatically")
                                          : qsTr("Manual start")
                            checked: window.backend.autostart
                            onToggled: window.backend.setAutostart(checked)
                        }
                    }
                }
            }

            Frame {
                Layout.fillWidth: true
                padding: 16
                background: Rectangle {
                    radius: 14
                    color: Qt.rgba(1.0, 0.70, 0.0, 0.09)
                    border.color: Qt.rgba(1.0, 0.70, 0.0, 0.35)
                }
                RowLayout {
                    width: parent.width
                    spacing: 12
                    Label {
                        text: "⚠"
                        font.pixelSize: 20
                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Themes can execute JavaScript. Only import themes from authors you trust.")
                        wrapMode: Text.WordWrap
                    }
                }
            }

            // About / uninstall ----------------------------------------------
            Frame {
                Layout.fillWidth: true
                Layout.bottomMargin: 24
                padding: 18

                background: Rectangle {
                    radius: 14
                    color: palette.base
                    border.color: palette.mid
                    border.width: 1
                }

                RowLayout {
                    width: parent.width
                    spacing: 12

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Label {
                            text: qsTr("Ultralight Web Cursor %1")
                                .arg(Qt.application.version)
                            font.bold: true
                        }
                        Label {
                            text: window.backend.canUninstall
                                ? qsTr("Installed with the Windows setup program")
                                : qsTr("Linux packages are managed by pacman / yay")
                            color: palette.placeholderText
                        }
                    }

                    Button {
                        visible: window.backend.canUninstall
                        text: qsTr("Uninstall…")
                        onClicked: uninstallDialog.open()
                    }
                }
            }
        }
    }
}
