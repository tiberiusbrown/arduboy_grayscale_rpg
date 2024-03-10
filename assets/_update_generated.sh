#/bin/bash

set -e

cd "$(dirname "$0")"

shopt -s expand_aliases
alias python=python3

python ../scripts/convert_map.py
python ../scripts/convert_sprites.py
python ../scripts/convert_game_over_messages.py
python ../scripts/convert_items.py
python ../scripts/convert_midi.py
python ../scripts/font.py
python ../scripts/save_tile_solid.py

cd ../arduboy_build
python ../scripts/gen_fxsave.py
python fxdata-build.py fxdata.txt
mv fxdata.h ../src/generated/fxdata.h
python ../scripts/gen_fxheader.py

