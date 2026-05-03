@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >/dev/null
set PATH=C:\Program Files\CMake\bin;%PATH%
cd /d "C:\Users\商克谦\OneDrive\Desktop\DeepSeekPlugin\build"
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
