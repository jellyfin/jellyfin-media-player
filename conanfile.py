from conans import ConanFile, CMake
import os

class PlexMediaPlayer(ConanFile):
  settings = "os", "compiler", "build_type", "arch"
  options = {"include_desktop": [True, False]}
  default_options = "include_desktop=True"
  generators = "cmake"

  def requirements(self):
    self.requires("web-client-tv/2.12.7-4d3ceb0@plex/public")

    if self.options.include_desktop:
      self.requires("web-client-desktop/3.0.0-478e654@plex/public")
