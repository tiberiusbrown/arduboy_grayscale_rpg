@echo off

cd /d %~dp0

call ..\assets\_update_generated.bat subcall
if NOT %errorlevel%==0 goto error

cd ..\src
set dir=%temp%/arduboy_grayscale_rpg_build
if not exist "%dir%" mkdir %dir%
arduino-cli.exe compile -v --log-level info ^
    -b arduboy:avr:arduboy . ^
    --output-dir "%dir%" ^
    --build-property compiler.c.elf.extra_flags="-Wl,--relax" ^
    --build-property compiler.c.extra_flags="-mcall-prologues -DARDUBOY_NO_USB" ^
    --build-property compiler.cpp.extra_flags="{compiler.c.extra_flags}"
if NOT %errorlevel%==0 goto error

rem copy hex file to dir
cd ..\arduboy_build
echo F|xcopy /S /Q /Y /F "%dir%/src.ino.hex" "arduboy_grayscale_rpg.hex" > nul
if NOT %errorlevel%==0 goto error

rem create _asm.txt file
"C:\Program Files (x86)\Arduino\hardware\tools\avr\bin\avr-objdump.exe" -S "%dir%/src.ino.elf" > _asm.txt

rem create _sizes.txt
"C:\Program Files (x86)\Arduino\hardware\tools\avr\bin\avr-nm.exe" --size-sort -C -r -t d "%dir%/src.ino.elf" > _sizes.txt

rem create _ram.txt
findstr /c:" b " /c:" B " /c:" d " /c:" D " _sizes.txt > _ram.txt

rem create _map.txt
"C:\Program Files (x86)\Arduino\hardware\tools\avr\bin\avr-nm.exe" --numeric-sort -C -t x "%dir%/src.ino.elf" > _map2.txt
findstr /c:" b " /c:" B " /c:" d " /c:" D " _map2.txt > _map.txt
del _map2.txt

rem create arduboy file
tar -a -cf arduboy_grayscale_rpg.zip arduboy_grayscale_rpg.hex fxdata.bin info.json LICENSE.txt banner_700x192.png banner_128x64.png
if NOT %errorlevel%==0 goto error
move /y arduboy_grayscale_rpg.zip ..\arduboy_grayscale_rpg.arduboy > nul
if NOT %errorlevel%==0 goto error

python flashcart-builder.py flashcart-index.csv
if NOT %errorlevel%==0 goto error

goto end
:error
pause
exit /b 1
:end

timeout 10
