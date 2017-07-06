from conans import ConanFile, CMake
import os

class PlexMediaPlayer(ConanFile):
  settings = "os", "compiler", "build_type", "arch"
  options = {"include_desktop": [True, False]}
  default_options = "include_desktop=True"
  generators = "cmake"

  def requirements(self):
    self.requires("web-client-tv2/3.12.0-a094a251@plex/stable")

    if self.options.include_desktop:
      self.requires("web-client-desktop/3.10.0-efd1768@plex/public")
