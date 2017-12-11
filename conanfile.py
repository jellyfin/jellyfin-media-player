from conans import ConanFile, CMake
import os

class PlexMediaPlayer(ConanFile):
  settings = "os", "compiler", "build_type", "arch"
  options = {"include_desktop": [True, False]}
  default_options = "include_desktop=True"
  generators = "cmake"

  def requirements(self):
    self.requires("web-client-tv2/3.30.0-22f9886a@plex/public")

    if self.options.include_desktop:
      self.requires("web-client-desktop/3.29.7-98ffa9d@plex/public")
