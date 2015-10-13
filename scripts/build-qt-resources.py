#!/usr/bin/python -u
#
# This creates a Qt resources XML file.
#
# Usage:
#
#  build-qt-resources.py output.qrc virtualpath1=realpath1 virtualpath2=realpath2 ... virtualpathN=realpathN
#
# "output.qrc" will be overwritten! Each argument after the output file
# consists of a virtual path, which defines the resource prefix, and a real
# path (i.e. an actual file system path), which sets the source file or
# directory. If the real path is a file, the virtual path sets the full
# virtual file name. If the real path is a directory, the directory is
# scanned recursively, and all files are added, using the virtual path as
# prefix.

import sys
import os
import stat
import io

if len(sys.argv) < 2:
  sys.exit(1)

result = "<RCC>\n"

def add_files(virtualpath, filepath):
  global result

  fstat = os.stat(filepath)
  if stat.S_ISDIR(fstat.st_mode):
    for item in os.listdir(filepath):
      add_files(os.path.join(virtualpath, item), os.path.join(filepath, item))
  else:
    dirname, fname = os.path.split(virtualpath)
    result += (('<qresource prefix=\"%s\">\n' +
                ' <file alias=\"%s\">%s</file>\n' +
                '</qresource>\n') % (dirname, fname, filepath))

for item in sys.argv[2:]:
  virtualpath, filepath = item.split("=", 1)
  add_files(virtualpath, filepath)

result += "</RCC>"

result = result.encode("utf8")

try:
  with open(sys.argv[1], "rb") as infile:
    if infile.read() == result:
      # They're the same -> prevent cmake from rebuilding resources.
      sys.exit(0)
except IOError:
  pass

outfile = open(sys.argv[1], "wb")
outfile.write(result)
