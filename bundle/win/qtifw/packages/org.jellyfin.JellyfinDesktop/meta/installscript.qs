function Component()
{
    component.loaded.connect(this, Component.prototype.loaded);

    if (installer.isUninstaller()) {
        // Connect to uninstallation finished to handle user data cleanup
        installer.uninstallationFinished.connect(this, Component.prototype.onUninstallFinished);
    }
}

Component.prototype.loaded = function()
{
    if (installer.isInstaller()) {
        // Add shortcut options to target directory page
        installer.addWizardPageItem(component, "ShortcutOptions", QInstaller.TargetDirectory);

        // Load previous install folder from Qt IFW's uninstall registry
        try {
            var result = installer.execute("reg", ["query",
                "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{97f7667b-2fc9-4608-9ef0-a5dd22464f92}",
                "/v", "InstallLocation"]);
            if (result && result[0]) {
                var match = result[0].match(/InstallLocation\s+REG_SZ\s+(.+)/);
                if (match && match[1]) {
                    var prevPath = match[1].trim();
                    if (prevPath.length > 0) {
                        installer.setValue("TargetDir", prevPath);
                    }
                }
            }
        } catch (e) {
            // Registry key doesn't exist, use default
        }
    }

    if (installer.isUninstaller()) {
        // Add uninstall options to introduction page
        installer.addWizardPageItem(component, "UninstallOptions", QInstaller.Introduction);
    }
}

Component.prototype.onUninstallFinished = function()
{
    var widget = component.userInterface("UninstallOptions");
    var removeUserData = widget ? widget.removeUserData.checked : false;

    if (removeUserData) {
        var appData = installer.environmentVariable("APPDATA");
        var localAppData = installer.environmentVariable("LOCALAPPDATA");

        // Remove user data directories
        installer.execute("cmd", ["/c", "rmdir", "/s", "/q", appData + "\\Jellyfin Desktop"]);
        installer.execute("cmd", ["/c", "rmdir", "/s", "/q", localAppData + "\\Jellyfin Desktop"]);
    }
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
        var targetDir = installer.value("TargetDir");
        var exePath = targetDir + "\\Jellyfin Desktop.exe";

        // Install VC++ Redistributable (downloads if needed, skips if already installed)
        var arch = systemInfo.currentCpuArchitecture === "arm64" ? "arm64" : "x64";
        var vcRedistUrl = "https://aka.ms/vs/17/release/vc_redist." + arch + ".exe";
        var vcRedistPath = installer.value("TargetDir") + "\\vc_redist.exe";

        // Download VC++ Redistributable
        component.addOperation("Execute",
            "powershell", "-Command",
            "Invoke-WebRequest -Uri '" + vcRedistUrl + "' -OutFile '" + vcRedistPath + "'",
            "errormessage=Failed to download VC++ Redistributable");

        // Install VC++ Redistributable (silent, no restart)
        component.addElevatedOperation("Execute",
            vcRedistPath, "/install", "/quiet", "/norestart",
            "errormessage=Failed to install VC++ Redistributable");

        // Clean up vc_redist installer
        component.addOperation("Execute",
            "cmd", "/c", "del", "/q", vcRedistPath);

        // Get checkbox states
        var widget = component.userInterface("ShortcutOptions");
        var createStartMenu = widget ? widget.createStartMenu.checked : true;
        var createDesktop = widget ? widget.createDesktop.checked : true;

        // Start Menu shortcut
        if (createStartMenu) {
            component.addOperation("CreateShortcut",
                exePath,
                "@StartMenuDir@/Jellyfin Desktop.lnk",
                "workingDirectory=" + targetDir,
                "iconPath=" + exePath,
                "iconId=0",
                "description=Jellyfin desktop client");
        }

        // Desktop shortcut
        if (createDesktop) {
            component.addOperation("CreateShortcut",
                exePath,
                "@DesktopDir@/Jellyfin Desktop.lnk",
                "workingDirectory=" + targetDir,
                "iconPath=" + exePath,
                "iconId=0",
                "description=Jellyfin desktop client");
        }
    }
}
