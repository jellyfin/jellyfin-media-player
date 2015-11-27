// Skip all pages and go directly to finished page.
// (see also componenterror example)
function cancelInstaller(message)
{
  installer.setDefaultPageVisible(QInstaller.Introduction, false);
  installer.setDefaultPageVisible(QInstaller.TargetDirectory, false);
  installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
  installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
  installer.setDefaultPageVisible(QInstaller.StartMenuSelection, false);
  installer.setDefaultPageVisible(QInstaller.PerformInstallation, false);
  installer.setDefaultPageVisible(QInstaller.LicenseCheck, false);

  var abortText = "<font color='red'>" + message +"</font>";
  installer.setValue("FinishedText", abortText);
}

function isVersionNewer(version, version2) {

  if (version && version2) {
    version = version.split('.');
    version2 = version2.split('.');

    for (var i = 0, len = version.length; i < len; i++) {
      if (i < version2.length) {
        var minVersionDot = parseInt(version[i], 10);
        var version2Dot = parseInt(version2[i], 10);

        if (minVersionDot < version2Dot) {
          return true;
        } else if (minVersionDot > version2Dot) {
          return false;
        }
      }
    }
    return true;
  } else {
    return true;
  }
}

function Component()
{
  // start installer with -v to see debug output
  console.log("OS: " + systemInfo.productType);
  console.log("Kernel: " + systemInfo.kernelType + "/" + systemInfo.kernelVersion);

  var validOs = false;
  var validArch = false;

  if (systemInfo.kernelType === "winnt") {

    // check for windows 7 or newer
    if (isVersionNewer("6.1.0", systemInfo.kernelVersion))
      validOs = true;

    if (systemInfo.currentCpuArchitecture === "x86_64")
      validArch = true;
  }

  if (!validOs || !validArch) {
    cancelInstaller("Installation on " + systemInfo.prettyProductName + " (" + systemInfo.currentCpuArchitecture + ") is not supported");
  }
}

Component.prototype.createOperations = function()
{
  component.createOperations();

  if (systemInfo.productType === "windows") {
    component.addOperation("CreateShortcut", "@TargetDir@/PlexMediaPlayer.exe", "@StartMenuDir@/Plex Media Player.lnk");
    component.addOperation("CreateShortcut", "@TargetDir@/maintenancetool.exe", "@StartMenuDir@/Maintain Plex Media Player.lnk");
  }
}
