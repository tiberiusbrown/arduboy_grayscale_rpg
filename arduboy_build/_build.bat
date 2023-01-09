@echo off

cd /d %~dp0

call ..\assets\_update_generated.bat subcall
if NOT %errorlevel%==0 goto error

rem set "remove_timer0=-D__vector_23=__attribute__((naked,weak))remove_timer0 -DREMOVE_TIMER0"
set "remove_timer0=-DREMOVE_TIMER0"
rem set "remove_timer0=-DDEBUG_BUILD"
rem set "remove_timer0=-DDEBUG_MONOCHROME"

cd ..\src
set dir=%temp%/arduboy_grayscale_rpg_build
if not exist "%dir%" mkdir %dir%
arduino-cli.exe compile -v --log-level info ^
    -b arduboy-homemade:avr:arduboy-fx . ^
    --output-dir "%dir%" ^
    --build-property compiler.c.elf.extra_flags="-Wl,--relax" ^
    --build-property compiler.c.extra_flags="-fshort-enums -frename-registers -fno-tree-scev-cprop -mcall-prologues -mstrict-X %remove_timer0%" ^
    --build-property compiler.cpp.extra_flags="{compiler.c.extra_flags}"
if NOT %errorlevel%==0 goto error

rem copy hex file to dir
cd ..\arduboy_build
echo F|xcopy /S /Q /Y /F "%dir%/src.ino.hex" "ReturnOfTheArdu.hex" > nul
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
tar -a -cf ReturnOfTheArdu.zip ReturnOfTheArdu.hex fxdata.bin fxsave.bin info.json LICENSE.txt banner_700x192.png banner_128x64.png
if NOT %errorlevel%==0 goto error
move /y ReturnOfTheArdu.zip ..\ReturnOfTheArdu.arduboy > nul
if NOT %errorlevel%==0 goto error

python flashcart-builder.py flashcart-index.csv
if NOT %errorlevel%==0 goto error

goto end
:error
pause
exit /b 1
:end

timeout 10
