from conans import ConanFile, CMake
import os

class PlexMediaPlayer(ConanFile):
  settings = "os", "compiler", "build_type", "arch"
  options = {"include_desktop": [True, False]}
  default_options = "include_desktop=True"
  generators = "cmake"

  def requirements(self):
    self.requires("web-client-tv2/3.19.2-cff0356a@plex/stable")

    if self.options.include_desktop:
      self.requires("web-client-desktop/3.14.1-762b124@plex/public")
