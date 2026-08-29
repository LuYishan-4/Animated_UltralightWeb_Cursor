import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

ApplicationWindow {
    id: window
    visible: true
    width: 1080
    height: 760
    title: qsTr("Web Cursor")

    Material.theme: Material.Dark
    Material.accent: Material.DeepPurple

    Uistaff {
        anchors.fill: parent
        backend: appBackend
    }
}
