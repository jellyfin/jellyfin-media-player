function Component()
{
  // default constructor
}

Component.prototype.createOperations = function()
{
  component.createOperations();

  if (systemInfo.productType === "windows") {
    component.addOperation("CreateShortcut", "@TargetDir@/PlexMediaPlayer.exe", "@StartMenuDir@/Plex Media Player.lnk");
    component.addOperation("CreateShortcut", "@TargetDir@/PlexMediaPlayer-angle.bat", "@StartMenuDir@/Plex Media Player (DirectX).lnk");
    component.addOperation("CreateShortcut", "@TargetDir@/maintenancetool.exe", "@StartMenuDir@/Maintain Plex Media Player.lnk");
  }
}