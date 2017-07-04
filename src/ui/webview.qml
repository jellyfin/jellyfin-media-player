import QtQuick 2.4
import Konvergo 1.0
import QtWebEngine 1.1
import QtWebChannel 1.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.4

KonvergoWindow
{
  id: mainWindow
  title: "Plex Media Player"
  objectName: "mainWindow"
  minimumHeight: windowMinSize.height
  minimumWidth: windowMinSize.width

  function runWebAction(action)
  {
    if (mainWindow.webDesktopMode)
      web.triggerWebAction(action)
  }

  Action
  {
    enabled: mainWindow.webDesktopMode
    shortcut: {
      if (components.system.isMacos) return "Ctrl+Shift+F";
      return "F11";
    }
    onTriggered: mainWindow.toggleFullscreen()
  }

  Action
  {
    enabled: mainWindow.webDesktopMode
    shortcut:
    {
      if (components.system.isMacos) return "Ctrl+Meta+F"
      return "Shift+F11"
    }
    onTriggered: mainWindow.toggleFullscreenNoSwitch()
  }

  Action
  {
    shortcut: "Alt+Return"
    enabled:
    {
      if (mainWindow.webDesktopMode && components.system.isWindows)
        return true;
      return false;
    }
    onTriggered: mainWindow.toggleFullscreen()
  }

  Action
  {
    enabled: mainWindow.webDesktopMode
    shortcut: StandardKey.Close
    onTriggered: mainWindow.close()
  }

  Action
  {
    enabled: mainWindow.webDesktopMode
    shortcut: {
      if (components.system.isMacos) return "Ctrl+M";
      return "Meta+Down";
    }
    onTriggered: mainWindow.minimizeWindow()
  }

  Action
  {
    enabled: mainWindow.webDesktopMode
    shortcut: StandardKey.Quit
    onTriggered: mainWindow.close()
  }

  Action
  {
    shortcut: "Ctrl+Shift+D"
    enabled: mainWindow.webDesktopMode
    onTriggered: mainWindow.toggleDebug()
  }

  Action
  {
    shortcut: StandardKey.Copy
    onTriggered: runWebAction(WebEngineView.Copy)
    id: action_copy
  }

  Action
  {
    shortcut: StandardKey.Cut
    onTriggered: runWebAction(WebEngineView.Cut)
    id: action_cut
  }

  Action
  {
    shortcut: StandardKey.Paste
    onTriggered: runWebAction(WebEngineView.Paste)
    id: action_paste
  }

  Action
  {
    shortcut: StandardKey.SelectAll
    onTriggered: runWebAction(WebEngineView.SelectAll)
    id: action_selectall
  }

  Action
  {
    shortcut: StandardKey.Undo
    onTriggered: runWebAction(WebEngineView.Undo)
    id: action_undo
  }

  Action
  {
    shortcut: StandardKey.Redo
    onTriggered: runWebAction(WebEngineView.Redo)
    id: action_redo
  }

  MpvVideo
  {
    id: video
    objectName: "video"
    // It's not a real item. Its renderer draws onto the view's background.
    width: 0
    height: 0
    visible: false
  }

  WebEngineView
  {
    id: web
    objectName: "web"
    settings.errorPageEnabled: false
    settings.localContentCanAccessRemoteUrls: true
    profile.httpUserAgent: components.system.getUserAgent()
    transformOrigin: Item.TopLeft
    url: mainWindow.webUrl
    focus: true
    property string currentHoveredUrl: ""
    onLinkHovered: web.currentHoveredUrl = hoveredUrl
    width: mainWindow.width
    height: mainWindow.height
    scale: 1

    Component.onCompleted:
    {
      // set the transparency
      // (setting this here as a UserAgent workaround at least for qt5.5)
      backgroundColor : "#111111"
      forceActiveFocus()
      mainWindow.reloadWebClient.connect(reload)
    }

    onLoadingChanged:
    {
      // we use a timer here to switch to the webview since
      // it take a few moments for the webview to render
      // after it has loaded.
      //
      if (loadRequest.status == WebEngineView.LoadStartedStatus)
      {
        console.log("WebEngineLoadRequest starting: " + loadRequest.url);
      }
      else if (loadRequest.status == WebEngineView.LoadSucceededStatus)
      {
        console.log("WebEngineLoadRequest success: " + loadRequest.url);
      }
      else if (loadRequest.status == WebEngineView.LoadFailedStatus)
      {
        console.log("WebEngineLoadRequest failure: " + loadRequest.url + " error code: " + loadRequest.errorCode);
        errorLabel.visible = true
        errorLabel.text = "Error loading client, this is bad and should not happen<br>" +
                          "You can try to <a href='reload'>reload</a> or head to our <a href='http://plex.tv/support'>support page</a><br><br>Actual Error: <pre>" +
                          loadRequest.errorString + " [" + loadRequest.errorCode + "]</pre><br><br>" +
                          "Provide the <a target='_blank' href='file://"+ components.system.logFilePath + "'>logfile</a> as well."
      }
    }

    onNewViewRequested:
    {
      if (request.userInitiated)
      {
        console.log("Opening external URL: " + web.currentHoveredUrl)
        components.system.openExternalUrl(web.currentHoveredUrl)
      }
    }

    onFullScreenRequested:
    {
      console.log("Request fullscreen: " + request.toggleOn)
      mainWindow.setFullScreen(request.toggleOn)
      request.accept()
    }

    onJavaScriptConsoleMessage:
    {
      components.system.info(message)
    }

    onCertificateError:
    {
      console.log(error.url + " :" + error.description + error.error)
    }
  }

  Text
  {
    id: errorLabel
    z: 5
    anchors.centerIn: parent
    color: "#999999"
    linkColor: "#cc7b19"
    text: "Generic error"
    font.pixelSize: 32
    font.bold: true
    visible: false
    verticalAlignment: Text.AlignVCenter
    textFormat: Text.StyledText
    onLinkActivated:
    {
      if (link == "reload")
      {
        errorLabel.visible = false
        web.reload()
      }
      else
      {
        Qt.openUrlExternally(link)
      }
    }
  }


  Rectangle
  {
    id: debug
    color: "black"
    z: 10
    anchors.centerIn: parent
    width: parent.width
    height: parent.height
    opacity: 0.7
    visible: mainWindow.showDebugLayer

    Text
    {
      id: debugLabel
      width: (parent.width - 50) / 2
      height: parent.height - 25
      anchors.left: parent.left
      anchors.leftMargin: 64
      anchors.top: parent.top
      anchors.topMargin: 54
      anchors.bottomMargin: 54
      color: "white"
      font.pixelSize: Math.round(height / 65)
      wrapMode: Text.WrapAnywhere

      function windowDebug()
      {
        var dbg = mainWindow.debugInfo + "Window and web\n";
        dbg += "  Window size: " + parent.width + "x" + parent.height + " - " + web.width + "x" + web.height + "\n";
        dbg += "  DevicePixel ratio: " + Screen.devicePixelRatio + "\n";

        return dbg;
      }

      text: windowDebug()
    }

    Text
    {
      id: videoLabel
      width: (parent.width - 50) / 2
      height: parent.height - 25
      anchors.right: parent.right
      anchors.left: debugLabel.right
      anchors.rightMargin: 64
      anchors.top: parent.top
      anchors.topMargin: 54
      anchors.bottomMargin: 54
      color: "white"
      font.pixelSize: Math.round(height / 65)
      wrapMode: Text.WrapAnywhere

      text: mainWindow.videoInfo
    }
  }

  property QtObject webChannel: web.webChannel
}
