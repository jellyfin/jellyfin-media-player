#!/usr/bin/env python
# Sample CI usage: scripts\common\fetch-binaries.py -p windows -b release -t abc123 -n 33
# Sample desktop usage: scripts\common\fetch-binaries.py -i ..\plex-dependency-builder\output\Packages\pms-depends-windows-i386-debug-dev.bz2

import hashlib
import optparse
import ConfigParser
import os, re
import platform
import subprocess
import sys
import shutil
import urllib2
import base64
import glob

# Edit these to set a new default dependencies build
default_tag = "auto"
default_release_build_number = "101"
default_release_dir = "plexmediaplayer-dependencies"
default_branch = "master"

def sha1_for_file(path):
    hash=hashlib.sha1()
    fp=file(path, "rb")
    while True:
        data=fp.read(4096)
        if not data:
            break
        hash.update(data)
    return hash.hexdigest()

def exec_cmd(args, env={}, supress_output=False):
    """ main exec_cmd function """

    # forward SSH_AUTH_SOCK, so that ssh-agent works
    if os.name != "nt" and "SSH_AUTH_SOCK" in os.environ:
        env = os.environ
        extra_env={"SSH_AUTH_SOCK":os.environ["SSH_AUTH_SOCK"]}
        env.update(extra_env)
    else:
        env = os.environ

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


platform_map={"linux-synology-i386":"synology-i686",
              "linux-readynas-arm":"ubuntu-arm",
              "linux-debian-4-i386":"debian-i686",
              "linux-control4-arm":"control4-arm",
              "linux-apm-ppc":"apm-ppc",
              "linux-armada-arm7":"armada-arm7",
              "linux-synology-arm":"synology-arm"}

def platform_str():
    if "BUILD_TAG" in os.environ:
        for (k, v) in platform_map.iteritems():
            if k in os.environ["BUILD_TAG"]:
                return "linux-"+v

    return "linux-%s-%s"%(platform.linux_distribution()[0].strip().lower(), platform.machine())

def merge_directories(src, dest, move = False):
    for src_dir, dirs, files in os.walk(src):
        dst_dir = src_dir.replace(src, dest)
        if not os.path.exists(dst_dir):
            os.mkdir(dst_dir)
        for file_ in files:
            src_file = os.path.join(src_dir, file_)
            dst_file = os.path.join(dst_dir, file_)
            if os.path.exists(dst_file):
                os.remove(dst_file)
            if move:
                shutil.move(src_file, dst_dir)
            else:
                shutil.copy(src_file, dst_dir)
    if move and os.path.exists(src):
        shutil.rmtree(src)

def unpack_and_install(download, inputfile, installed_filepath):
    # Make paths absolute before changing directories
    inputfile = os.path.abspath(inputfile)
    if not os.path.exists(inputfile):
        print "Input file %s does not exist" % inputfile
        sys.exit(1)
    shafile = "%s.sha.txt" % inputfile

    installed_filepath = os.path.abspath(installed_filepath)

    # Go to directory.
    old_cwd = os.getcwd()
    os.chdir(options.output)

    # Check the SHA
    if options.nochecksha:
        print "-- Skipping SHA verification"
    elif os.path.exists(shafile):
        f = open(shafile, "r")
        sha = f.readline().strip()
        computed = sha1_for_file(inputfile)
        if not computed == sha:
            print "-- SHA didn't match: %s != %s" % (sha, computed)
            sys.exit(1)
        else:
            print "-- SHA %s matches" % sha

        f.close()
    else:
        print "-- ERROR - No SHA file is available to verify the file's integrity"
        sys.exit(1)

    # Untar the package file
    inputfile_tarfriendly = inputfile
    if os.name == "nt":
        pattern = re.compile(r'([a-z]):\\', re.IGNORECASE)
        inputfile_tarfriendly = pattern.sub('/\\1/', inputfile).replace('\\','/')

    # The final destination directory is the filename without a version number
    # The version number is the last element in the filename (by convention)
    packagename = os.path.splitext(os.path.basename(inputfile))[0]
    packagename_elements = packagename.split("-")
    del packagename_elements[-1]
    packagename_noversion = "-".join(packagename_elements)

    if os.path.exists(packagename_noversion):
        shutil.rmtree(packagename_noversion, ignore_errors=True)

    os.makedirs(packagename_noversion)

    print "-- Unpacking %s... to %s" % (os.path.basename(inputfile), packagename_noversion)
    if os.name == "nt":
        # Touch files from the package, which often arrive from the future when freshly built on our Windows build machine
        exec_cmd(["tar", "xjf", inputfile_tarfriendly, "-C", packagename_noversion, "--touch", "--strip-components", "1", "--no-same-owner"])
    else:
        exec_cmd(["tar", "xjf", inputfile_tarfriendly, "-C", packagename_noversion, "--strip-components", "1", "--no-same-owner"])

    if download and installed_filepath:
        # Create the installed stamp file to note our success
        open(installed_filepath, "wb")
    
    # Restore directory.
    os.chdir(old_cwd)
        
    return packagename

if __name__=='__main__':
    parser=optparse.OptionParser()

    parser.add_option("-p", "--platform", action="store", type="string",
        dest="platform", help="Platform identifier (e.g. windows-i386)", default=None)

    parser.add_option("-b", "--buildconfig", action="store", type="string",
        dest="buildconfig", help="Build configuration (release or debug). Default is release", default="release")

    parser.add_option("-t", "--tag", action="store", type="string",
        dest="tag", help="Build tag. Default is %s" % default_tag, default=default_tag)

    parser.add_option("-n", "--buildnumber", action="store", type="string",
        dest="buildnumber", help="Build number. Default is %s for release builds" % (default_release_build_number), default=None)

    parser.add_option("-d", "--dir", action="store", type="string",
        dest="dir", help="CI build dir. Default is %s for release builds" % (default_release_dir), default=None)

    parser.add_option("-i", "--inputfile", action="store", type="string",
        dest="inputfile", help="Dependencies package filename, can be supplied instead of platform, buildconfig, tag and buildnumber", default=None)

    parser.add_option("-o", "--output", action="store", type="string",
        dest="output", help="Output directory. Default is Dependencies",
        default="Dependencies")

    parser.add_option("-x", "--nochecksha", action="store_true",
        dest="nochecksha", help="Don't check the SHA. Default is false",
        default=False)

    parser.add_option("-r", "--branch", action="store", type="string",
            dest="branch", help="Git branch", default=None)

    (options, args)=parser.parse_args(sys.argv)

    if not os.path.exists(options.output):
        os.makedirs(options.output)

    # Fail early if platform is not known
    download = not options.inputfile
    if download and not options.platform:
        print "ERROR - A platform must be specified"
        sys.exit(1)

    installed_filepath = None
    if download:

        if not options.buildnumber:
            if options.buildconfig == "release":
                options.buildnumber = default_release_build_number
            else:
                options.buildnumber = default_debug_build_number

        if not options.dir:
            if options.buildconfig == "release":
                options.dir = default_release_dir
            else:
                options.dir = default_debug_dir

        installed_filepath = os.path.join(options.output, "konvergo-depends-%s-%s-%s.installed" % (options.platform, options.buildconfig, options.buildnumber))
        if os.path.exists(installed_filepath) and options.buildnumber != "latest":
            print "The required deps bundle was already downloaded and installed."
            print "You can delete %s to force a reinstall." % installed_filepath
            sys.exit(0)

        # Delete previous installed stamps
        cont = ["-c"]
        for path in glob.iglob(os.path.join(options.output, "konvergo-depends-%s-%s-*.installed") % (options.platform, options.buildconfig)):
            match = re.search("-(\d+)\.installed", path, re.DOTALL)
            if match:
                if not match.group(1) == options.buildnumber:
                    cont = []
            os.remove(path)

        if options.tag == "auto":
            req = urllib2.Request("https://nightlies.plex.tv/directdl/plex-dependencies/%s/%s/hash.txt" % (options.dir, options.buildnumber))
            try:
                match = urllib2.urlopen(req).read().rstrip()
            except urllib2.URLError, err:
                print err
                print "ERROR - Download failed"
                sys.exit(1)

            options.tag = match

        base_filename = "konvergo-depends-%s-%s-%s" % (options.platform, options.buildconfig, options.tag)
        filename = "%s.tbz2" % base_filename

        installed_filepath = os.path.join(options.output, "konvergo-depends-%s-%s-%s.installed" % (options.platform, options.buildconfig, options.buildnumber))
        if os.path.exists(installed_filepath):
            print "%s was already downloaded and installed." % filename
            print "You can delete %s to force a reinstall." % installed_filepath
            sys.exit(0)

        url = "https://nightlies.plex.tv/directdl/plex-dependencies/%s/%s/%s" % (options.dir, options.buildnumber, filename)
        inputfile = os.path.join(options.output, filename)

        print "-- Downloading %s ..." % url
        exec_cmd(["wget", "--no-check-certificate"] + cont + ["-O", inputfile, url])

        shaurl = "%s.sha.txt" % url
        shafile = "%s.sha.txt" % inputfile

        print "-- Downloading %s ..." % shaurl
        exec_cmd(["wget", "--no-check-certificate"] + ["-O", shafile, shaurl])

    else:
        inputfile = options.inputfile

    # Unpack and install
    packagename = unpack_and_install(download, inputfile, installed_filepath)

    # On OS X, we need to postprocess the dependencies.
    if platform.system() == 'Darwin':
        root = os.path.realpath(os.path.join(os.getcwd()))
        script = os.path.join(root, "scripts", "fix-install-names.py")
        for p in ("lib", "bin", "update_installer"):
            path = os.path.join(root, "Dependencies", "konvergo-depends-" + options.platform + "-" + options.buildconfig, p)
            exec_cmd([script, path])

    # Done!
    print "-- Done with %s" % packagename
