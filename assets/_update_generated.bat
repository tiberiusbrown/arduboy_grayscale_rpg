@echo off

cd /d %~dp0

python ..\scripts\convert_map.py
if NOT %errorlevel%==0 goto error
python ..\scripts\convert_sprites.py
if NOT %errorlevel%==0 goto error
python ..\scripts\convert_game_over_messages.py
if NOT %errorlevel%==0 goto error
python ..\scripts\convert_items.py
if NOT %errorlevel%==0 goto error
python ..\scripts\convert_midi.py
if NOT %errorlevel%==0 goto error
python ..\scripts\font.py
if NOT %errorlevel%==0 goto error

cd ..\arduboy_build
python ..\scripts\gen_fxsave.py
if NOT %errorlevel%==0 goto error
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
