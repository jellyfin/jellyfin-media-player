from conans import ConanFile, CMake
import os

class PlexMediaPlayer(ConanFile):
  settings = "os", "compiler", "build_type", "arch"
  options = {"include_desktop": [True, False]}
  default_options = "include_desktop=True"
  generators = "cmake"

  def requirements(self):
    self.requires("web-client-tv2/3.16.0-87b18d1a@plex/stable")

    if self.options.include_desktop:
      self.requires("web-client-desktop/3.9.1-85417e0@plex/public")
