from pathlib import Path
import sys
import os
import argparse
from collections import deque
import subprocess
import shutil

def main(argv=tuple(sys.argv[1:])):
  arg_parser = argparse.ArgumentParser(description='Fix third party library paths in .app bundles.')
  arg_parser.add_argument('bundle', metavar='BUNDLE', type=str, nargs=1)
  arguments = arg_parser.parse_args(argv)

  bundle_path = Path(arguments.bundle[0])
  print(bundle_path)
  webengine_path = bundle_path / 'Contents' / 'Frameworks' / 'QtWebEngineCore.framework' / 'Helpers' / 'QtWebEngineProcess.app'
  link_path = webengine_path / 'Contents' / 'Frameworks'
  bin_to_fix = webengine_path / 'Contents' / 'MacOS' / 'QtWebEngineProcess'

  os.symlink('../../../../../../../Frameworks', link_path)

  result = subprocess.check_output(['otool', '-L', str(bin_to_fix.resolve())],stderr=subprocess.STDOUT).decode('utf-8')
  for dependency in result.splitlines():
    dependency = dependency.strip().lstrip()
    if dependency.startswith('/opt/homebrew') or dependency.startswith('/usr/local'):
      # cut off trailing compatibility string
      dependency_str = dependency.split(' (compatibility')[0].strip()
      dependency_framework_str = dependency_str.split('/lib')[1].strip()
      dependency_framework = Path(dependency_framework_str)
      target = f'@executable_path/../Frameworks{dependency_framework_str}'
      subprocess.run(['install_name_tool', '-id', target, bin_to_fix])
      subprocess.run(['install_name_tool', '-change', dependency_str, target, bin_to_fix])

if __name__ == '__main__':
  main()
