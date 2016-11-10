from conans import ConanFile, CMake
import os

class PlexMediaPlayer(ConanFile):
  settings = "os", "compiler", "build_type", "arch"
  requires = (
    "web-client-desktop/2.10.8-d0ff548@plex/stable",
    "web-client-tv/2.10.8-cbaa73c@plex/stable"
  )
  
  def imports(self):
    self.copy("*", dst="web-client/tv", src="tv")
    self.copy("*", dst="web-client/desktop", src="desktop")
