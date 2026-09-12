// Standalone settings window.
//
// Layout and styling are ported from the caelestia-kde-plugins
// `web-cursor-settings` panel; the Quickshell plugin plumbing (shell palette,
// SDK/build bootstrap, pkexec install, KWin D-Bus) is replaced by this
// project's `SettingsBackend` exposed as `appBackend`.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
    id: window

    visible: true
    width: 900
    height: 780
    minimumWidth: 520
    minimumHeight: 460
    title: qsTr("Web Cursor")
    color: uwcTheme.background

    property var backend: appBackend

    Colors { id: uwcTheme }

    FolderDialog {
        id: themeImportDialog
        title: qsTr("Choose a cursor theme folder")
        onAccepted: {
            var path = selectedFolder.toString().replace(/^file:\/\//, "");
            if (window.backend)
                window.backend.uploadTheme(path);
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ---- Header ------------------------------------------------------
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: Style.paddingLarge
            Layout.leftMargin: Style.paddingXLarge
            spacing: Style.spacingMedium

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0
                Text {
                    text: qsTr("Web Cursor")
                    font.family: Style.fontFamilyHeading
                    font.pixelSize: Style.fontTitleLarge
                    font.weight: Font.DemiBold
                    color: uwcTheme.surfaceText
                }
                Text {
                    text: qsTr("HTML/CSS cursor rendered by Ultralight")
                    font.family: Style.fontFamily
                    font.pixelSize: Style.fontCaption
                    color: uwcTheme.surfaceVariantText
                }
            }
        }

        // Decorative banner (drawn, so the app needs no artwork assets).
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 140
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: uwcTheme.primaryContainer }
                GradientStop { position: 1.0; color: uwcTheme.tertiaryContainer }
            }
        }

        // ---- Scrollable content -----------------------------------------
        Flickable {
            id: contentFlick
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: width
            contentHeight: contentLayout.implicitHeight + Style.paddingXLarge
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
                parent: contentFlick
                anchors.top: contentFlick.top
                anchors.right: contentFlick.right
                anchors.bottom: contentFlick.bottom
                active: contentFlick.contentHeight > contentFlick.height
                visible: active
                contentItem: Rectangle {
                    implicitWidth: 6
                    radius: 3
                    color: uwcTheme.primary
                }
            }

            ColumnLayout {
                id: contentLayout
                width: contentFlick.width - Style.paddingXLarge * 2
                x: Style.paddingXLarge
                y: Style.paddingMedium
                spacing: Style.spacingMedium

                // ---- Enable ----------------------------------------------
                RowCard {
                    Layout.fillWidth: true
                    colors: uwcTheme
                    SwitchRow {
                        id: enableRow
                        anchors.fill: parent
                        text: qsTr("Enable Web Cursor")
                        subtext: qsTr("Render the selected HTML/CSS cursor across the desktop")
                        checked: window.backend ? window.backend.enabled : false
                        colors: uwcTheme
                        onToggled: on => {
                            if (!window.backend) return;
                            on ? window.backend.enable() : window.backend.disable();
                        }
                    }
                    implicitHeight: enableRow.implicitHeight + padding * 2
                }

                // ---- Theme picker ----------------------------------------
                RowLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: Style.spacingLarge

                    SectionHeader {
                        Layout.fillWidth: true
                        text: qsTr("Cursor Theme")
                        colors: uwcTheme
                        first: true
                    }
                    IconButton {
                        icon: "add"
                        colors: uwcTheme
                        tonal: true
                        onClicked: themeImportDialog.open()
                    }
                }

                RowCard {
                    Layout.fillWidth: true
                    colors: uwcTheme
                    RowLayout {
                        id: currentThemeRow
                        anchors.fill: parent
                        spacing: Style.spacingLarge

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 0
                            Text {
                                text: qsTr("Current theme")
                                font.family: Style.fontFamily
                                font.pixelSize: Style.fontCaption
                                color: uwcTheme.surfaceVariantText
                            }
                            Text {
                                Layout.fillWidth: true
                                text: window.backend ? window.backend.currentTheme : ""
                                font.family: Style.fontFamily
                                font.pixelSize: Style.fontBodyLarge
                                color: uwcTheme.surfaceText
                                elide: Text.ElideRight
                            }
                        }
                        IconButton {
                            icon: "refresh"
                            colors: uwcTheme
                            onClicked: { if (window.backend) window.backend.reload(); }
                        }
                    }
                    implicitHeight: currentThemeRow.implicitHeight + padding * 2
                }

                Repeater {
                    model: window.backend ? window.backend.themeList : []

                    delegate: RowCard {
                        required property string modelData
                        readonly property var details: window.backend
                            ? window.backend.getThemeDetails(modelData)
                            : ({})

                        Layout.fillWidth: true
                        colors: uwcTheme
                        implicitHeight: themeRow.implicitHeight + padding * 2

                        RowLayout {
                            id: themeRow
                            anchors.fill: parent
                            spacing: Style.spacingLarge

                            Image {
                                readonly property real baseSize: 56
                                Layout.preferredWidth: baseSize
                                Layout.preferredHeight: baseSize
                                Layout.alignment: Qt.AlignVCenter
                                sourceSize.width: 112
                                sourceSize.height: 112
                                source: details.iconPath || ""
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                                cache: false
                                visible: status === Image.Ready
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                Text {
                                    Layout.fillWidth: true
                                    text: modelData
                                    font.family: Style.fontFamily
                                    font.pixelSize: Style.fontBody
                                    color: uwcTheme.surfaceText
                                    elide: Text.ElideRight
                                }
                                Text {
                                    Layout.fillWidth: true
                                    visible: (details.describe || "").length > 0
                                    text: details.describe || ""
                                    font.family: Style.fontFamily
                                    font.pixelSize: Style.fontCaption
                                    color: uwcTheme.surfaceVariantText
                                    elide: Text.ElideRight
                                }
                                Text {
                                    text: qsTr("By %1 · minimum %2 × %3")
                                        .arg(details.author || qsTr("Unknown"))
                                        .arg(details.minWidth || 128)
                                        .arg(details.minHeight || 128)
                                    font.family: Style.fontFamily
                                    font.pixelSize: Style.fontTiny
                                    color: uwcTheme.surfaceVariantText
                                }
                            }

                            IconButton {
                                readonly property bool active: window.backend
                                    && window.backend.currentTheme === modelData
                                icon: active ? "check" : "play_arrow"
                                colors: uwcTheme
                                onClicked: {
                                    if (window.backend) window.backend.useTheme(modelData);
                                }
                            }
                            IconButton {
                                icon: "folder_open"
                                colors: uwcTheme
                                onClicked: {
                                    if (window.backend) window.backend.openThemeFolder(modelData);
                                }
                            }
                            IconButton {
                                icon: "delete"
                                colors: uwcTheme
                                onClicked: {
                                    if (window.backend) window.backend.removeTheme(modelData);
                                }
                            }
                        }
                    }
                }

                // ---- Size ------------------------------------------------
                SectionHeader {
                    Layout.fillWidth: true
                    text: qsTr("Size")
                    colors: uwcTheme
                }

                RowCard {
                    Layout.fillWidth: true
                    colors: uwcTheme
                    StepperRow {
                        id: widthRow
                        anchors.fill: parent
                        text: qsTr("Cursor width")
                        subtext: qsTr("Render width in pixels")
                        min: 1
                        max: 1920
                        value: window.backend ? window.backend.cursorWidth : 128
                        colors: uwcTheme
                        onMoved: value => {
                            if (window.backend) window.backend.cursorWidth = value;
                        }
                    }
                    implicitHeight: widthRow.implicitHeight + padding * 2
                }

                RowCard {
                    Layout.fillWidth: true
                    colors: uwcTheme
                    StepperRow {
                        id: heightRow
                        anchors.fill: parent
                        text: qsTr("Cursor height")
                        subtext: qsTr("Render height in pixels")
                        min: 1
                        max: 1080
                        value: window.backend ? window.backend.cursorHeight : 128
                        colors: uwcTheme
                        onMoved: value => {
                            if (window.backend) window.backend.cursorHeight = value;
                        }
                    }
                    implicitHeight: heightRow.implicitHeight + padding * 2
                }

                // ---- Rendering -------------------------------------------
                SectionHeader {
                    Layout.fillWidth: true
                    text: qsTr("Rendering")
                    colors: uwcTheme
                }

                RowCard {
                    Layout.fillWidth: true
                    colors: uwcTheme
                    SwitchRow {
                        id: gpuRow
                        anchors.fill: parent
                        text: qsTr("GPU rendering")
                        subtext: qsTr("Use hardware-accelerated compositing when available")
                        checked: window.backend ? window.backend.gpuRender : true
                        colors: uwcTheme
                        onToggled: on => {
                            if (window.backend) window.backend.gpuRender = on;
                        }
                    }
                    implicitHeight: gpuRow.implicitHeight + padding * 2
                }

                RowCard {
                    Layout.fillWidth: true
                    colors: uwcTheme
                    SwitchRow {
                        id: autostartRow
                        anchors.fill: parent
                        text: qsTr("Launch on startup")
                        subtext: qsTr("Automatically start the cursor engine after login")
                        checked: window.backend ? window.backend.autostart : false
                        colors: uwcTheme
                        onToggled: on => {
                            if (window.backend) window.backend.setAutostart(on);
                        }
                    }
                    implicitHeight: autostartRow.implicitHeight + padding * 2
                }

                // ---- Ignored applications --------------------------------
                SectionHeader {
                    Layout.fillWidth: true
                    text: qsTr("Ignored Applications")
                    colors: uwcTheme
                }

                RowCard {
                    Layout.fillWidth: true
                    colors: uwcTheme
                    ColumnLayout {
                        id: blacklistColumn
                        anchors.fill: parent
                        spacing: Style.spacingSmall

                        Repeater {
                            model: window.backend ? window.backend.blacklist : []

                            delegate: RowLayout {
                                required property string modelData
                                Layout.fillWidth: true

                                Text {
                                    Layout.fillWidth: true
                                    text: modelData
                                    font.family: Style.fontFamily
                                    font.pixelSize: Style.fontBody
                                    color: uwcTheme.surfaceText
                                    elide: Text.ElideRight
                                }
                                IconButton {
                                    icon: "close"
                                    colors: uwcTheme
                                    onClicked: {
                                        if (window.backend) window.backend.removeBlacklist(modelData);
                                    }
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Style.spacingSmall

                            TextField {
                                id: blacklistInput
                                Layout.fillWidth: true
                                placeholderText: qsTr("Window class or application name")
                                color: uwcTheme.surfaceText
                                placeholderTextColor: uwcTheme.surfaceVariantText
                                selectByMouse: true
                                font.family: Style.fontFamily
                                font.pixelSize: Style.fontBody
                                background: Rectangle {
                                    radius: Style.radiusMedium
                                    color: uwcTheme.surface
                                    border.color: uwcTheme.outline
                                    border.width: 1
                                }
                                onAccepted: addBlacklistFromInput()
                            }
                            IconButton {
                                icon: "add"
                                colors: uwcTheme
                                tonal: true
                                onClicked: addBlacklistFromInput()
                            }
                        }
                    }
                    implicitHeight: blacklistColumn.implicitHeight + padding * 2
                }

                // ---- Status ----------------------------------------------
                Text {
                    Layout.fillWidth: true
                    visible: window.backend && window.backend.statusMessage.length > 0
                    text: window.backend ? window.backend.statusMessage : ""
                    color: uwcTheme.surfaceVariantText
                    font.family: Style.fontFamily
                    font.pixelSize: Style.fontCaption
                    wrapMode: Text.WordWrap
                }
            }
        }
    }

    function addBlacklistFromInput() {
        if (!window.backend) return;
        const value = blacklistInput.text.trim();
        if (value.length === 0) return;
        window.backend.addBlacklist(value);
        blacklistInput.clear();
    }
}
