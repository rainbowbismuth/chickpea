#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

BAKE=tools/bake/target/debug/bake

# Tile swizzle patterns
CHARACTER="01\
           01\
           23\
           23"

PORTRAIT="000011\
          000011\
          000011\
          000011\
          000022\
          000022\
          000022\
          000022\
          333344\
          333344"

$BAKE 4bpp -i assets/fonts/CGA8x8thin.png -o baked/fonts/debug_font
$BAKE font -i assets/fonts/bismuth.png -o baked/fonts/bismuth_font
$BAKE 4bpp -i assets/map/tile_highlight.png -o baked/map/tile_highlight
$BAKE 4bpp -i assets/map/tile_cursor.png -o baked/map/tile_cursor
$BAKE 4bpp -i assets/map/tile_pointer.png -o baked/map/tile_pointer
$BAKE 4bpp -s $CHARACTER -i assets/characters/bjin.png -o baked/characters/bjin
$BAKE map -i assets/map/demo/map -t assets/map/demo/tileset.png -o baked/map/demo/map
$BAKE bg -i assets/interface/speech_bubble.png -o baked/interface/speech_bubble
$BAKE 8bpp -i assets/portraits/Bjin.png -s $PORTRAIT --offset=6 -o baked/portraits/Bjin

pushd baked > /dev/null
echo "" > baked.c
for file in **/*.* ; do
  xxd -i $file >> baked.c
done
for file in map/**/*.* ; do
  xxd -i $file >> baked.c
done
popd > /dev/null
