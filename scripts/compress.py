#!/usr/bin/env python -u

# simple script that takes data on stdin and outputs bz2 compressed data
# just to avoid having to install bzip2.exe on windows platforms

import bz2, os, sys

bufsize=4096

if __name__ == "__main__":
  compress = bz2.BZ2Compressor()
  data = sys.stdin.read(bufsize)
  while data:
    cdata = compress.compress(data)
    if cdata:
      sys.stdout.write(cdata)
    data = sys.stdin.read(bufsize)

  cdata = compress.flush()
  if cdata:
    sys.stdout.write(cdata)
