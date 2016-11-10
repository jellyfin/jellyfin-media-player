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

  function actionEnable(enable)
  {
    action_escape.enabled = enable
    action_switchmode.enabled = enable
    action_copy.enabled = enable
    action_cut.enabled = enable
    action_paste.enabled = enable
    action_undo.enabled = enable
    action_redo.enabled = enable
    action_selectall.enabled = enable
  }

  Action
  {
    id: action_escape
    shortcut: "Escape"
    onTriggered:
    {
      if (mainWindow.fullScreen)
        mainWindow.setFullScreen(false)
    }
  }

  Action
  {
    id: action_switchmode
    shortcut: "Ctrl+M"
    onTriggered:
    {
      if (mainWindow.webDesktopMode)
        components.settings.cycleSetting("main.webMode")
    }
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

  function maxWebScale()
  {
    return webHeightMax ? ((webHeightMax / Screen.devicePixelRatio) / 720) : 10;
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
    anchors.centerIn: parent
    settings.errorPageEnabled: false
    settings.localContentCanAccessRemoteUrls: true
    profile.httpUserAgent: components.system.getUserAgent()
    transformOrigin: Item.TopLeft
    url: mainWindow.webUrl
    focus: true
    property string currentHoveredUrl: ""
    onLinkHovered: web.currentHoveredUrl = hoveredUrl

    width:
    {
      if (!mainWindow.webDesktopMode)
      {
        return Math.round(Math.min((parent.height * 16) / 9, parent.width));
      }
      else
      {
        return parent.width;
      }
    }

    height:
    {
      if (!mainWindow.webDesktopMode)
      {
        return Math.round(Math.min((parent.width * 9) / 16, parent.height));
      }
      else
      {
        return parent.height;
      }

    }
    
    scale:
    {
      if (mainWindow.webDesktopMode)
        return 1;

      if (mainWindow.windowScale < mainWindow.maxWebScale())
      {
        // Web renders at windows scale, no scaling
        return 1;
      }
      else
      {
        // Web should max out at maximum scaling
        return mainWindow.windowScale / mainWindow.maxWebScale();
      }
    }

    Component.onCompleted:
    {
      // set the transparency
      // (setting this here as a UserAgent workaround at least for qt5.5)
      backgroundColor : "#111111"
      forceActiveFocus()
      mainWindow.reloadWebClient.connect(reload)
      actionEnable(mainWindow.webDesktopMode)
    }

    onLoadingChanged:
    {
      // we use a timer here to switch to the webview since
      // it take a few moments for the webview to render
      // after it has loaded.
      //
      if (loadRequest.status == WebEngineView.LoadSucceededStatus)
      {
        console.log("Loaded web-client successfully from: " + web.url);
      }
      else if (loadRequest.status == WebEngineView.LoadFailedStatus)
      {
        console.log("FAILED TO LOAD web-client successfully from: " + web.url);
        errorLabel.visible = true
        errorLabel.text = "Error loading client, this is bad and should not happen<br>" +
                          "You can try to <a href='reload'>reload</a> or head to our <a href='http://plex.tv/support'>support page</a><br><br>Actual Error: <pre>" +
                          loadRequest.errorString + " [" + loadRequest.errorCode + "]</pre><br><br>" +
                          "Provide the <a target='_blank' href='file://"+ components.system.logFilePath + "'>logfile</a> as well."
      }
      actionEnable(mainWindow.webDesktopMode)
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
      font.pixelSize: width / 45
      wrapMode: Text.WrapAnywhere

      function windowDebug()
      {
        var dbg = mainWindow.debugInfo + "Window and web\n";
        dbg += "  Window size: " + parent.width + "x" + parent.height + " - " + web.width + "x" + web.height + "\n";
        dbg += "  DevicePixel ratio: " + Screen.devicePixelRatio + "\n";
        dbg += "  Web Max Height: " + (webHeightMax / Screen.devicePixelRatio) + " / Max scale: " + mainWindow.maxWebScale() + "\n";
        dbg += "  Web scale: " + webScale + " / Window scale: " + windowScale + "\n";
        dbg += "  Scale applied: " + web.scale + "\n";

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
      font.pixelSize: width / 45
      wrapMode: Text.WrapAnywhere

      text: mainWindow.videoInfo
    }
  }

  property QtObject webChannel: web.webChannel
}
