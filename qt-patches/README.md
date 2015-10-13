In this directory we store all patches against Qt that is required by konvergo. The patches are:

* 0001-qtwebengine-Add-a-backgroundColor-property.patch - Which adds the backgroundColor property to
  the webengineview. Without this patch playback will not be rendered correctly. Has been submitted upstream
  and will most likely be included in Qt 5.6
* 0002-qtbase-Don-t-show-the-menu-bar-at-all-in-lion-style-fullscr.patch - This patch hids the menu bar
  when we go into fullscreen mode on OS X. Without this patch the menu bar can show up when it's not wanted
  (for example switching input on your TV). Has not been submitted upstream since it doesn't really fit
  for other applications.
* 0003-Always-enable-viewport-stuff.patch - This patch makes sure that the viewport is scalable on windows.
  Without this one the UI will be rendered in the upper left corner way to small. Has not been submitted
  upstream since it enables a feature that was intentionally disabled in desktop versions of Qt.
* 0004-qtwebengine-transparency-window-creation.patch - Fixes a bug in patch 0001. Will be included at the same
  time as patch 0001.