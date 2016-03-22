call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %PMP_VC_ARCH%
c:\msys32\mingw64\bin\gendef.exe - bin\mpv*.dll > bin\mpv-1.def
lib /def:bin\mpv-1.def /out:lib\mpv.lib /MACHINE:%PMP_LIB_ARCH%
