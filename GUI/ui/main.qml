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
                        "https://github.com/LuYishan-4/Cursor-animation"
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
            currentIndex: tabBar.currentIndex

            // =====================================================
            // Theme
            // =====================================================

            ColumnLayout {
                spacing: Kirigami.Units.largeSpacing

                Kirigami.Heading {
                    text: qsTr("Current Theme")
                    level: 3
                }

                ComboBox {
                    id: themeBox
                    Layout.fillWidth: true
                    model:   kcm.backend.themeList

                    currentIndex:
                          kcm.backend.themeList.indexOf(
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
                    Layout.fillWidth: true
                    Layout.preferredHeight: 300
                    clip: true
                    spacing: Kirigami.Units.smallSpacing
                    model:   kcm.backend.themeList

                    delegate: Kirigami.Card {
                        width: ListView.view.width

                        header:
                        Kirigami.Heading {
                            text: modelData
                            level: 4
                        }

                        contentItem:
                        RowLayout {
                            Button {
                                text: qsTr("Apply")
                                icon.name:
                                    "dialog-ok"

                                onClicked: {
                                      kcm.backend.useTheme(
                                        modelData
                                    )
                                }
                            }

                            Button {
                                text: qsTr("Open")

                                icon.name:
                                    "document-open-folder"

                                onClicked: {
                                      kcm.backend.openThemeFolder(
                                        modelData
                                    )
                                }
                            }

                            Button {
                                text: qsTr("Delete")

                                icon.name:
                                    "edit-delete"

                                onClicked: {
                                      kcm.backend.removeTheme(
                                        modelData
                                    )
                                }
                            }
                        }
                    }
                }

                Button {
                    text: qsTr("Upload Theme")

                    icon.name:
                        "folder-upload"

                    Layout.alignment:
                        Qt.AlignRight

                    onClicked:
                        themeUploadDialog.open()
                }
            }

            // =====================================================
            // Blacklist
            // =====================================================

            ColumnLayout {
                spacing:
                    Kirigami.Units.largeSpacing

                Kirigami.Heading {
                    text:
                        qsTr("Blacklist")

                    level: 3
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 300

                    model:
                          kcm.backend.blacklist

                    delegate:
                    Kirigami.Card {
                        width:
                            ListView.view.width

                        contentItem:
                        RowLayout {
                            Label {
                                text: modelData
                                Layout.fillWidth: true
                            }

                            Button {
                                text:
                                    qsTr("Remove")

                                icon.name:
                                    "edit-delete"

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

                        placeholderText:
                            qsTr("Application name")

                        Layout.fillWidth: true
                    }

                    Button {
                        text:
                            qsTr("Add")

                        onClicked: {
                            if (
                                blacklistInput.text.length > 0
                            ) {
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
                spacing:
                    Kirigami.Units.largeSpacing

                Kirigami.FormLayout {
                    Layout.fillWidth: true

                    TextField {
                        Kirigami.FormData.label:
                            qsTr("HTML Path")

                        text:
                              kcm.backend.htmlPath

                        onEditingFinished:
                              kcm.backend.htmlPath =
                                text
                    }

                    TextField {
                        Kirigami.FormData.label:
                            qsTr("SDK Path")

                        text:
                              kcm.backend.sdkPath

                        onEditingFinished:
                              kcm.backend.sdkPath =
                                text
                    }

                    SpinBox {
                        Kirigami.FormData.label:
                            qsTr("Cursor Width")

                        value:
                              kcm.backend.cursorWidth

                        from: 1
                        to: 512

                        onValueModified:
                              kcm.backend.cursorWidth =
                                value
                    }

                    SpinBox {
                        Kirigami.FormData.label:
                            qsTr("Cursor Height")

                        value:
                              kcm.backend.cursorHeight

                        from: 1
                        to: 512

                        onValueModified:
                              kcm.backend.cursorHeight =
                                value
                    }
                    CheckBox {
                        Kirigami.FormData.label:
                            qsTr("Auto Hide")

                        checked:
                              kcm.backend.autoHide

                        onToggled:
                              kcm.backend.autoHide =
                                checked
                    }
                }

                RowLayout {
                    Button {
                        text:
                            qsTr("Enable")

                        icon.name:
                            "media-playback-start"

                        onClicked:
                              kcm.backend.enable()
                    }

                    Button {
                        text:
                            qsTr("Disable")

                        icon.name:
                            "media-playback-stop"

                        onClicked:
                              kcm.backend.disable()
                    }

                    Button {
                        text:
                            qsTr("Reload")

                        icon.name:
                            "view-refresh"

                        onClicked:
                              kcm.backend.reconfigureKWin()
                    }
                }
            }
        }

        // =========================
        // Status
        // =========================

        Kirigami.InlineMessage {
            Layout.fillWidth: true

            visible:
                  kcm.backend.statusMessage.length > 0

            text:
                  kcm.backend.statusMessage

            type:
                Kirigami.MessageType.Information
        }
    }
}
