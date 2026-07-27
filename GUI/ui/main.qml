import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kcmutils

SimpleKCM {
    id: root

    title: qsTr("WebCursor")

    property int currentTab: 0

    FolderDialog {
        id: themeUploadDialog
        title: qsTr("Choose Theme Folder")

        onAccepted: {
            kcm.backend.uploadTheme(
                selectedFolder.toString().replace("file://", "")
            )
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.largeSpacing

        // =========================
        // Header
        // =========================

        RowLayout {
            Layout.fillWidth: true

            Kirigami.Heading {
                text: qsTr("Ultralight Web Cursor")
                level: 2
                Layout.fillWidth: true
            }

            Button {
                icon.name: "internet-services"
                text: qsTr("GitHub")

                onClicked: {
                    Qt.openUrlExternally(
                        "https://github.com/LuYishan-4/Animated_UltralightWeb_Cursor"
                    )
                }
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        // =========================
        // Tabs
        // =========================

        TabBar {
            id: tabBar
            Layout.fillWidth: true

            TabButton {
                text: qsTr("Theme")
            }

            TabButton {
                text: qsTr("Blacklist")
            }

            TabButton {
                text: qsTr("Setting")
            }
        }

           StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            ColumnLayout {
                spacing: Kirigami.Units.largeSpacing

                Kirigami.Heading {
                    text: qsTr("Current Theme")
                    level: 3
                }

                ComboBox {
                    id: themeBox
                    Layout.fillWidth: true
                    model: kcm.backend.themeList

                    currentIndex: kcm.backend.themeList.indexOf(
                        kcm.backend.currentTheme
                    )

                    onActivated: {
                        kcm.backend.useTheme(
                            currentText
                        )
                    }
                }

                Kirigami.Heading {
                    text: qsTr("Installed Themes")
                    level: 3
                }

          ListView {
    id: themeListView
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.minimumHeight: 250
    clip: true
    spacing: Kirigami.Units.mediumSpacing
    model: kcm.backend.themeList

    delegate: Kirigami.Card {
        id: themeCard
        width: ListView.view.width
        
     
        property var details: (modelData !== undefined && modelData !== "") ? kcm.backend.getThemeDetails(modelData) : null
        
        property string themeIcon: details ? (details.iconPath || "") : ""
        property string themeAuthor: details ? (details.author || "Unknown") : "Unknown"
        property string themeDesc: details ? (details.describe || "") : ""

        header: RowLayout {
            spacing: Kirigami.Units.largeSpacing
            
            Image {
                id: iconPreview
                source: themeCard.themeIcon ? themeCard.themeIcon : "image://theme/cursor"
                sourceSize.width: 48
                sourceSize.height: 48
                fillMode: Image.PreserveAspectFit
                Layout.alignment: Qt.AlignVCenter
                onStatusChanged: {
                    if (status === Image.Error) {
                        source = "image://theme/cursor"
                    }
                }
            }

            ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                Layout.fillWidth: true

                Kirigami.Heading {
                    text: modelData
                    level: 4
                    Layout.fillWidth: true
                }

                Label {
                    text: qsTr("Author: %1").arg(themeCard.themeAuthor)
                    font.italic: true
                    opacity: 0.6
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    Layout.fillWidth: true
                }
            }
        }
        
        contentItem: ColumnLayout {
            spacing: Kirigami.Units.largeSpacing

            Label {
                text: themeCard.themeDesc ? themeCard.themeDesc : qsTr("No description provided.")
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                opacity: 0.85
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                Button {
                    text: qsTr("Apply")
                    icon.name: "dialog-ok"
                    onClicked: kcm.backend.useTheme(modelData)
                }

                Button {
                    text: qsTr("Open")
                    icon.name: "document-open-folder"
                    onClicked: kcm.backend.openThemeFolder(modelData)
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: qsTr("Delete")
                    icon.name: "edit-delete"
                    onClicked: kcm.backend.removeTheme(modelData)
                }
            }
        }
    }
}

                Button {
                    text: qsTr("Upload Theme")
                    icon.name: "folder-upload"
                    Layout.alignment: Qt.AlignRight

                    onClicked:
                        themeUploadDialog.open()
                }
            }

            // =====================================================
            // Blacklist
            // =====================================================

            ColumnLayout {
                spacing: Kirigami.Units.largeSpacing

                Kirigami.Heading {
                    text: qsTr("Blacklist")
                    level: 3
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 200
                    clip: true
                    model: kcm.backend.blacklist

                    delegate: Kirigami.Card {
                        width: ListView.view.width

                        contentItem: RowLayout {
                            Label {
                                text: modelData
                                Layout.fillWidth: true
                            }

                            Button {
                                text: qsTr("Remove")
                                icon.name: "edit-delete"

                                onClicked: {
                                    kcm.backend.removeBlacklist(
                                        modelData
                                    )
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    TextField {
                        id: blacklistInput
                        placeholderText: qsTr("Application name")
                        Layout.fillWidth: true
                    }

                    Button {
                        text: qsTr("Add")

                        onClicked: {
                            if (blacklistInput.text.length > 0) {
                                kcm.backend.addBlacklist(
                                    blacklistInput.text
                                )

                                blacklistInput.clear()
                            }
                        }
                    }
                }
            }

            // =====================================================
            // Settings
            // =====================================================

            ColumnLayout {
                spacing: Kirigami.Units.largeSpacing

                Kirigami.FormLayout {
                    Layout.fillWidth: true

                    SpinBox {
                        Kirigami.FormData.label: qsTr("Cursor Width")
                        value: kcm.backend.cursorWidth
                        from: 1
                        to: 1920

                        onValueModified:
                            kcm.backend.cursorWidth = value
                    }

                    SpinBox {
                        Kirigami.FormData.label: qsTr("Cursor Height")
                        value: kcm.backend.cursorHeight
                        from: 1
                        to: 1080

                        onValueModified:
                            kcm.backend.cursorHeight = value
                    }
                }

                RowLayout {
                    Button {
                        text: qsTr("Enable")
                        icon.name: "media-playback-start"

                        onClicked:
                            kcm.backend.enable()
                    }

                    Button {
                        text: qsTr("Disable")
                        icon.name: "media-playback-stop"

                        onClicked:
                            kcm.backend.disable()
                    }

                    Button {
                        text: qsTr("Reload")
                        icon.name: "view-refresh"

                        onClicked:
                            kcm.backend.reconfigureKWin()
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }

        // =========================
        // Status
        // =========================

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: kcm.backend.statusMessage.length > 0
            text: kcm.backend.statusMessage
            type: Kirigami.MessageType.Information
        }
    }
}