#!/usr/bin/python

import hashlib
import optparse
import ConfigParser
import os
import platform
import subprocess
import sys
import shutil

exts = (".dylib", ".so")
exes = ("fc-cache", "macdeployqt", "qmake", "moc", "rcc", "qmlimportscanner")

def exec_cmd(args, env={}, supress_output=False):
    cmd = subprocess.Popen(args, stdout = subprocess.PIPE, stderr = subprocess.STDOUT, env = env)
    output = ''
    while True:
      out = cmd.stdout.read(1)
      if out == '' and cmd.poll() != None:
        break
      if out != '':
        if not supress_output:
          sys.stdout.write(out)
        output += out
    if cmd.wait() != 0:
      raise Exception("Command failed: \"%s\"" % " ".join(args), output)
    return output

def fix_install_name(path):
  for root, dirs, files in os.walk(path):
    for f in files:
      fpath = os.path.join(root, f)

      if (f.endswith(exts) or os.path.basename(f) in exes or (".framework/Versions/" in root and os.access(fpath, os.X_OK))) and not os.path.islink(fpath) and os.path.exists(fpath):
        # Fix permissions
        if not os.access(fpath, os.W_OK) or not os.access(fpath, os.R_OK) or not os.access(fpath, os.X_OK):
          os.chmod(fpath, 0o644)

        try:
          basename = os.path.basename(fpath)
          otoolout = exec_cmd(["otool", "-L", fpath], supress_output=True)

          for l in otoolout.split("\n")[1:]:
            l = l.rstrip().strip()

            # See if we need to fix it up.
            if len(l) > 0 and (l.startswith("/Users/admin/") or l[0] != '/'):
              current_lib = l.split(" (compat")[0]
              current_basename = os.path.basename(current_lib)
              correct_lib = os.path.join(root, current_basename)

              if not os.path.exists(correct_lib):
                # look for it further up, like in the root path:
                if os.path.exists(os.path.join(path, "lib", current_basename)):
                  correct_lib = os.path.join(path, "lib", current_basename)
                elif os.path.exists(os.path.join(path, "lib", current_lib)):
                  correct_lib = os.path.join(path, "lib", current_lib)
                else:
                  print "Can't link %s" % current_lib
                  continue

              # print current_lib, correct_lib
              if current_lib != correct_lib:
                if current_basename.split('.')[0] == basename.split('.')[0]:
                  print "-- Fixing ID for", basename
                  exec_cmd(["install_name_tool", "-id", correct_lib, fpath], supress_output=True)
                else:
                  print "-- Fixing library link for %s (%s)" % (basename, current_basename)
                  exec_cmd(["install_name_tool", "-change", current_lib, correct_lib, fpath], supress_output=True)

        except:
          print "** Fail when running installname on %s" % f
          raise
          continue

if __name__=='__main__':
  if os.path.isdir(sys.argv[1]):
    fix_install_name(sys.argv[1])
