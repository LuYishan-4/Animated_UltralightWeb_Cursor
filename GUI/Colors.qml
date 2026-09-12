// Palette used by the standalone settings UI.
//
// The original plugin bridged the Caelestia shell's material-3 palette through
// `qs.services`. The standalone app has no shell, so this provides the same
// short colour names backed by a dark material-3 baseline palette.
import QtQuick

QtObject {
    id: colors

    property color primary: "#a8c7fa"
    property color primaryText: "#00315a"
    property color primaryContainer: "#004a77"
    property color primaryContainerText: "#d1e4ff"
    property color primaryForeground: "#00315a"

    property color secondary: "#b8c7dc"
    property color secondaryText: "#233143"
    property color secondaryContainer: "#3a4859"
    property color secondaryContainerText: "#d4e3f8"

    property color tertiary: "#d0bcff"
    property color tertiaryText: "#37265b"
    property color tertiaryContainer: "#4e3d73"
    property color tertiaryContainerText: "#ebdcff"

    property color background: "#101418"
    property color backgroundText: "#e1e2e8"
    property color surface: "#191c20"
    property color surfaceText: "#e1e2e8"
    property color surfaceVariant: "#44474e"
    property color surfaceVariantText: "#c4c6cf"
    property color surfaceContainer: "#1d2024"

    property color error: "#ffb4ab"
    property color errorText: "#690005"
    property color errorContainer: "#93000a"
    property color errorContainerText: "#ffdad6"

    property color outline: "#8e9099"
    property color shadow: "#000000"
    property color inverseSurface: "#e1e2e8"
    property color inverseSurfaceText: "#2e3135"
    property color inversePrimary: "#3c5f96"
}
