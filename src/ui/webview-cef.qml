import QtQuick
import Konvergo 1.0
import Jellyfin.Cef 1.0
import QtWebChannel
import QtQuick.Window
import QtQuick.Controls

Window
{
  id: mainWindow
  title: "Jellyfin Desktop"
  objectName: "mainWindow"
  width: 1280
  height: 720
  minimumWidth: 213
  minimumHeight: 120
  visible: true
  color: "#000000"

  // Properties previously from KonvergoWindow
  property bool webDesktopMode: true
  property bool showDebugLayer: false
  property string debugInfo: ""
  property string videoInfo: ""
  property string webUrl: ""

  signal reloadWebClient()

  Component.onCompleted: {
    if (components && components.settings) {
      webUrl = components.settings.getWebClientUrl(webDesktopMode)
    }
  }

  function toggleFullscreen() {
    visibility = (visibility === Window.FullScreen) ? Window.Windowed : Window.FullScreen
  }

  function toggleDebug() {
    showDebugLayer = !showDebugLayer
  }

  function setFullScreen(enable) {
    visibility = enable ? Window.FullScreen : Window.Windowed
  }

  function minimizeWindow() {
    if (visibility !== Window.FullScreen)
      visibility = Window.Minimized
  }

  function runWebAction(action)
  {
    if (mainWindow.webDesktopMode)
      web.triggerWebAction(action)
  }

  Action
  {
    enabled: mainWindow.webDesktopMode
    shortcut:
    {
      if (components.system.isMacos) return "Ctrl+Meta+F"
      return "F11"
    }
    onTriggered: mainWindow.toggleFullscreen()
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
    onTriggered: runWebAction(16)
    id: action_copy
  }

  Action
  {
    shortcut: StandardKey.Cut
    onTriggered: runWebAction(17)
    id: action_cut
  }

  Action
  {
    shortcut: StandardKey.Paste
    onTriggered: runWebAction(18)
    id: action_paste
  }

  Action
  {
    shortcut: StandardKey.SelectAll
    onTriggered: runWebAction(22)
    id: action_selectall
  }

  Action
  {
    shortcut: StandardKey.Undo
    onTriggered: runWebAction(20)
    id: action_undo
  }

  Action
  {
    shortcut: StandardKey.Redo
    onTriggered: runWebAction(21)
    id: action_redo
  }

  Action
  {
    shortcut: StandardKey.Back
    onTriggered: runWebAction(0)
    id: action_back
  }

  Action
  {
    shortcut: StandardKey.Forward
    onTriggered: runWebAction(1)
    id: action_forward
  }

  Action
  {
    enabled: mainWindow.webDesktopMode
    shortcut: "Ctrl+0"
    onTriggered: web.zoomFactor = 1.0
  }

  WebChannel
  {
    id: webChannelObject
  }

  Binding
  {
    target: web
    property: "zoomFactor"
    value: 1.0
    when: !components.settings.allowBrowserZoom()
  }

  MpvVideoItem
  {
    id: video
    objectName: "video"
    enabled: true

    width: mainWindow.contentItem.width
    height: mainWindow.contentItem.height
    anchors.left: mainWindow.contentItem.left
    anchors.right: mainWindow.contentItem.right
    anchors.top: mainWindow.contentItem.top

    Component.onCompleted: {
      console.log("MpvVideoItem size:", width, "x", height, "visible:", visible)
    }
    onWidthChanged: console.log("MpvVideoItem width changed:", width)
    onHeightChanged: console.log("MpvVideoItem height changed:", height)
  }

  CefView
  {
    id: web
    objectName: "web"
    anchors.fill: parent
    z: 100
    backgroundColor: "transparent"

    // this is needed to prevent intermittent(?) black screens when unminizing
    // or resumsing from suspend (linux/{x11/wayland}, possibly others).
    layer.enabled: true

    webChannel: webChannelObject
    userAgent: components.system.getUserAgent()
    storagePath: components.profileManager ? components.profileManager.activeProfile.dataDir("CEF") : ""
    settingsJson: components.system.getSettingsJson()
    url: mainWindow.webUrl
    focus: true

    onFullScreenRequested: function(toggleOn) {
      mainWindow.setFullScreen(toggleOn)
    }

    Component.onCompleted:
    {
      console.log("CefView size:", width, "x", height, "backgroundColor:", backgroundColor)
      forceActiveFocus()
      mainWindow.reloadWebClient.connect(reload)

      // Handle CSP workaround from C++
      components.system.pageContentReady.connect(function(html, finalUrl, hadCSP) {
        if (hadCSP) {
          console.log("CSP workaround: navigating to", finalUrl);
          web.url = finalUrl;
        }
      })

      // Inject nativeshell script
      web.injectScript(components.system.getNativeShellScript());
    }

    onLoadingChanged: function(url, status, errorCode, errorString)
    {
      // we use a timer here to switch to the webview since
      // it take a few moments for the webview to render
      // after it has loaded.
      //
      if (status == 0) // LoadStartedStatus
      {
        console.log("CefView LoadRequest starting: " + url);
        // Inject nativeshell script early - before page scripts run
        web.injectScript(components.system.getNativeShellScript());
      }
      else if (status == 1) // LoadSucceededStatus
      {
        console.log("CefView LoadRequest success: " + url);
      }
      else if (status == 2) // LoadFailedStatus
      {
        console.log("CefView LoadRequest failure: " + url + " error code: " + errorCode);
        errorLabel.visible = true
        errorLabel.text = "Error loading client, this is bad and should not happen<br>" +
                          "You can try to <a href='reload'>reload</a> or head to our <a href='http://jellyfin.org'>support page</a><br><br>Actual Error: <pre>" +
                          errorString + " [" + errorCode + "]</pre><br><br>" +
                          "Provide the <a target='_blank' href='file://"+ components.system.logFilePath + "'>logfile</a> as well."
      }
    }

    onJavaScriptConsoleMessage: function(level, message, lineNumber, sourceID)
    {
      components.system.jsLog(level, sourceID + ":" + lineNumber + " " + message);
    }
  }

  Text
  {
    id: errorLabel
    z: 5
    anchors.centerIn: parent
    color: "#999999"
    linkColor: "#a85dc3"
    text: "Generic error"
    font.pixelSize: 32
    font.bold: true
    visible: false
    verticalAlignment: Text.AlignVCenter
    textFormat: Text.StyledText
    onLinkActivated:
    {
      if (url == "reload")
      {
        errorLabel.visible = false
        web.reload()
      }
      else
      {
        Qt.openUrlExternally(url)
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
