function Controller()
{
    // Set ProductUUID for consistent Windows registry entries across reinstalls
    // (not supported as config.xml element in Qt IFW 4.7)
    installer.setValue("ProductUUID", "{97f7667b-2fc9-4608-9ef0-a5dd22464f92}");

    // Simplify wizard - hide unnecessary pages
    // Must be done in Controller (not Component) so it applies before first page shows
    // Only for installer - uninstaller needs Introduction page for UninstallOptions widget
    if (installer.isInstaller()) {
        installer.setDefaultPageVisible(QInstaller.Introduction, false);
    }
    installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
    installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
    installer.setDefaultPageVisible(QInstaller.StartMenuSelection, false);
    installer.setDefaultPageVisible(QInstaller.LicenseCheck, false);
    // Keep: TargetDirectory, PerformInstallation, FinishedPage

    // Handle silent installation
    if (installer.isCommandLineInstance()) {
        installer.autoRejectMessageBoxes();
        installer.setMessageBoxAutomaticAnswer("OverwriteTargetDirectory", QMessageBox.Yes);
        installer.setMessageBoxAutomaticAnswer("stopProcessesForUpdates", QMessageBox.Ignore);
    }
}

Controller.prototype.TargetDirectoryPageCallback = function()
{
    if (installer.isCommandLineInstance()) {
        gui.clickButton(buttons.NextButton);
    }
}

Controller.prototype.FinishedPageCallback = function()
{
    if (installer.isCommandLineInstance()) {
        gui.clickButton(buttons.FinishButton);
    }
}
