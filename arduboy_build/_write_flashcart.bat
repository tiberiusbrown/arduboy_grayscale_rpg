
cd /d %~dp0

python flashcart-writer.py flashcart-image.bin

if NOT %errorlevel%==0 pause
