#!/usr/bin/python
import subprocess, os, sys, shutil, Queue, fnmatch

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

binaries = {}

def install_libraries(binary):
  if binaries.has_key(binary): return []
  binaries[binary] = True
  other_binaries = []

  out = exec_cmd(["otool", "-L", binary], supress_output=True)
  for line in out.split('\n'):
    line = line.strip()
    if line.find("dylib") != -1 and (line.startswith('@executable_path') or line.find("libQt6") != -1):
      file = line.split('/')[-1].split(' ')[0]
      if line.startswith("@executable_path"):
        src = os.path.join(sys.argv[1], "dependencies/darwin-x86_64/lib", file)
      else:
        src = line.split(" (")[0]

      dst = os.path.join(sys.argv[2], "Konvergo.app", "Contents", "Frameworks", file)
      if os.path.exists(src):
        if (not os.path.exists(dst) or os.path.getsize(dst) != os.path.getsize(src)):
          print "-- Installing %s" % file
          shutil.copyfile(src, dst)

        other_binaries.append(src)

  return other_binaries

queue = Queue.Queue()

# Add the top-level binaries.
for b in ['Contents/MacOS/Konvergo']:
  queue.put(os.path.join(sys.argv[2], "Konvergo.app", b))

for root, dirs, files in os.walk(os.path.join(sys.argv[2], "Konvergo.app", "Contents", "Frameworks", "vlc")):
  for f in files:
    path = os.path.join(root, f)
    queue.put(path)

while queue.empty() == False:
  binary = queue.get()
  other_binaries = install_libraries(binary)
  for b in other_binaries:
    queue.put(b)