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

echo "#include \"game/resource.h\"" > baked/baked.c

$BAKE 4bpp -i assets/fonts/CGA8x8thin.png -o fonts_debug_font >> baked/baked.c
$BAKE font -i assets/fonts/bismuth.png -o fonts_bismuth_font >> baked/baked.c
$BAKE 4bpp -i assets/map/tile_highlight.png -o map_tile_highlight >> baked/baked.c
$BAKE 4bpp -i assets/map/tile_cursor.png -o map_tile_cursor >> baked/baked.c
$BAKE 4bpp -i assets/map/tile_pointer.png -o map_tile_pointer >> baked/baked.c
$BAKE 4bpp -s $CHARACTER -i assets/characters/bjin.png -o characters_bjin >> baked/baked.c
$BAKE map -i assets/map/demo/map -t assets/map/demo/tileset.png -o map_demo_map >> baked/baked.c
$BAKE bg -i assets/interface/speech_bubble.png -o interface_speech_bubble >> baked/baked.c
$BAKE 8bpp -i assets/portraits/Bjin.png -s $PORTRAIT --offset=6 -o portraits_Bjin >> baked/baked.c
