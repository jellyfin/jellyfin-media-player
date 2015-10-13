import QtQuick 2.2
import QtQuick.Dialogs 1.1

MessageDialog {
  id: messageDialog
  title: errorTitle
  text: errorTitle
  informativeText: errorText
  icon: StandardIcon.Critical
  standardButtons: StandardButton.Help | StandardButton.Close

  onRejected: {
    Qt.quit()
  }

  onHelp: {
    Qt.openUrlExternally("https://forums.plex.tv")
    Qt.quit()
  }

  Component.onCompleted: visible = true
}
