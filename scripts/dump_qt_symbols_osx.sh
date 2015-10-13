#!/bin/sh

set -e

rm -rf symbols
mkdir symbols

dylibs=$(find lib plugins -type f -name *.dylib)
frameworks=$(find lib -type d -name *.framework)

echo "Dumping symbols for dylibs..."
for lib in $dylibs; do
  libname=$(basename $lib)
  echo $libname
  dsymutil -o symbols/$libname.dSYM $lib &>/dev/null
  ~/dump_syms -g symbols/$libname.dSYM $lib 2> /dev/null | xz -9 -e - 1> symbols/$libname.symbols.xz
done

for f in $frameworks; do
  frameworkname=$(basename $f)
  frameworkfname="${frameworkname%.*}"
  ffname="$f/$frameworkfname"
  if [ -e $ffname ]; then
    echo $frameworkname
    dsymutil -o symbols/$frameworkname.dSYM $ffname
    ~/dump_syms -g symbols/$frameworkname.dSYM $ffname 2>/dev/null | xz -9 -e - > symbols/$frameworkname.symbols.xz
  fi
done
