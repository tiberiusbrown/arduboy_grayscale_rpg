@echo off

cd /d %~dp0

python ..\scripts\convert_font.py
if NOT %errorlevel%==0 goto error
python ..\scripts\convert_sprites.py
if NOT %errorlevel%==0 goto error
python ..\scripts\convert_map.py
if NOT %errorlevel%==0 goto error

cd ..\arduboy_build
python fxdata-build.py fxdata.txt
if NOT %errorlevel%==0 goto error
move /y fxdata.h ..\src\generated\fxdata.h > nul
if NOT %errorlevel%==0 goto error
python ..\scripts\gen_fxheader.py
if NOT %errorlevel%==0 goto error

goto end
:error
pause
exit /b 1
:end

if "%1" neq "subcall" timeout 10
