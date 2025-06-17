from pathlib import Path
import sys
import argparse
from collections import deque
import subprocess
import shutil
import platform


def main(argv=tuple(sys.argv[1:])):
    arg_parser = argparse.ArgumentParser(description='Fix third party library paths in .app bundles.')
    arg_parser.add_argument('bundle', metavar='BUNDLE', type=str, nargs=1)
    arguments = arg_parser.parse_args(argv)

    bundle_path = Path(arguments.bundle[0])
    framework_path = bundle_path / 'Contents' / 'Frameworks'
    rpath_str = '/usr/local/lib'
    if platform.machine() == 'arm64':
        rpath_str = '/opt/homebrew/lib'
    framework_libs = set(file.name for file in framework_path.glob('*.dylib')).union(set(file.name for file in framework_path.glob('*.so')))
    libs_to_fix = deque()
    libs_to_fix.extend(file for file in bundle_path.glob('**/*.dylib'))
    libs_to_fix.extend(file for file in bundle_path.glob('**/*.so'))
    while libs_to_fix:
        lib = libs_to_fix.popleft()
        # find the dependencies of the library
        result = subprocess.check_output(['otool', '-L', str(lib.resolve())], stderr=subprocess.STDOUT).decode('utf-8')
        for dependency in result.splitlines():
            dependency = dependency.strip().lstrip()
            if dependency.startswith('/usr/local') or dependency.startswith('/opt/homebrew') or dependency.startswith('@rpath') or dependency.startswith('@loader_path'):
                # cut off trailing compatibility string
                dependency_str = dependency.split(' (compatibility')[0].strip()
                if dependency.startswith('@rpath'):
                    dependency = dependency_str.replace("@rpath", rpath_str)
                    dependency = Path(dependency)
                elif dependency.startswith('@loader_path'):
                    dependency = dependency_str.replace("@loader_path", rpath_str)
                    dependency = Path(dependency)
                    dependency_name = dependency.name
                    dependency = (Path(rpath_str) / dependency_name).resolve()
                else:
                    dependency = Path(dependency_str)

                # if somehow macdeployqt didn't copy the lib for us, we do a manual copy
                if dependency.name not in framework_libs:
                    shutil.copy(str(dependency.resolve()), str((framework_path / dependency.name).resolve()))
                    framework_libs.add(dependency.name)
                    # add the newly added library in to the to fix queue
                    libs_to_fix.append(framework_path / dependency.name)
                    print((framework_path / dependency.name).resolve())
                    print(f'Copied {dependency} to {framework_path / dependency.name}')

                # now we fix the path using install_name_tool
                target = f'@executable_path/../Frameworks/{dependency.name}'
                print(f'Fixing dependency {dependency_str} of {lib} to {target}')
                subprocess.run(['install_name_tool', '-id', target, lib])
                subprocess.run(['install_name_tool', '-change', dependency_str, target, lib])

    # Explicitly fix the main executable reference to @rpath/libdiscord-rpc.dylib
    main_exec = bundle_path / 'Contents' / 'MacOS' / 'Jellyfin Media Player'
    discordDylib = bundle_path / 'Contents' / 'Frameworks' / 'libdiscord-rpc.dylib'
    if main_exec.exists() and discordDylib.exists():
        print(f'Fixing @rpath/libdiscord-rpc.dylib in main executable: {main_exec}')
        subprocess.run([
            'install_name_tool',
            '-change',
            '@rpath/libdiscord-rpc.dylib',
            '@executable_path/../Frameworks/libdiscord-rpc.dylib',
            str(main_exec.resolve())
        ])

if __name__ == '__main__':
    main()
