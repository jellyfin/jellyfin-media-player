from conans import ConanFile, CMake
import os

class PlexMediaPlayer(ConanFile):
  settings = "os", "compiler", "build_type", "arch"
  options = {"include_desktop": [True, False]}
  default_options = "include_desktop=True"
  generators = "cmake"

  def requirements(self):
    self.requires("web-client-tv/3.3.0-70a4215@plex/public")

    if self.options.include_desktop:
      self.requires("web-client-desktop/3.5.0-e40051b@plex/public")
