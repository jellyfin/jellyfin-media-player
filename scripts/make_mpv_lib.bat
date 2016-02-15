@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
lib /def:bin\mpv-1.def /out:lib\mpv.lib /MACHINE:X64
