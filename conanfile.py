from conans import ConanFile, CMake
import os

class PlexMediaPlayer(ConanFile):
  settings = "os", "compiler", "build_type", "arch"
  options = {"include_desktop": [True, False]}
  default_options = "include_desktop=True"
  generators = "cmake"

  def requirements(self):
    self.requires("web-client-tv/2.10.8-fd540f9@plex/public")

    if self.options.include_desktop:
      self.requires("web-client-desktop/2.12.5-ac7ff53@plex/public")
