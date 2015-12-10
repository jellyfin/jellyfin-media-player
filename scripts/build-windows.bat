cd %BUILD_DIR%  || exit /b

%CMAKE_DIR%\cmake -DCRASHDUMP_SECRET=%CD_SECRET% -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=output .. -G Ninja -DCODE_SIGN=ON || exit /b

ninja || exit /b
ninja package || exit /b

..\scripts\WindowsSign.cmd PlexMediaPlayer-*.exe
