from conans import ConanFile, CMake
import os

class PlexMediaPlayer(ConanFile):
  settings = "os", "compiler", "build_type", "arch"
  options = {"include_desktop": [True, False]}
  default_options = "include_desktop=True"
  generators = "cmake"

  def requirements(self):
    self.requires("web-client-tv2/3.15.0-6e42625b@plex/stable")

    if self.options.include_desktop:
      self.requires("web-client-desktop/3.12.0-b3abe11@plex/public")
