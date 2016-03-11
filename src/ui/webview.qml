import QtQuick 2.4
import Konvergo 1.0
import QtWebEngine 1.1
import QtWebChannel 1.0
import QtQuick.Window 2.2

KonvergoWindow
{
  id: mainWindow
  title: "Plex Media Player"
  objectName: "mainWindow"
  minimumHeight: windowMinSize.height
  minimumWidth: windowMinSize.width

  function getInitialScaleArg()
  {
    return "?initialScale=" + webScale
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

    width: parent.width
    height: parent.height
    
    scale:
    {
      if (mainWindow.windowScale < mainWindow.maxWebScale()) {
        // Web renders at windows scale, no scaling
        return 1;
      } else {
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
      url = components.settings.value("path", "startupurl") + getInitialScaleArg();
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
        errorLabel.visible = true
        errorLabel.text = "Error loading client, this is bad and should not happen<br>" +
                          "You can try to <a href='reload'>reload</a> or head to our <a href='http://plex.tv/support'>support page</a><br><br>Actual Error: <pre>" +
                          loadRequest.url + "\n" + loadRequest.errorString + " [" + loadRequest.errorCode + "]</pre><br><br>" +
                          "Provide the <a href='file://"+ components.system.logFilePath() + "'>logfile</a> as well."
      }
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
        web.reload()
        errorLabel.visible = false
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
        dbg += "  Window size: " + parent.width + "x" + parent.height + "\n";
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
      wrapMode: Text.NoWrap

      text: mainWindow.videoInfo
    }
  }

  property QtObject webChannel: web.webChannel
}
