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

$BAKE 4bpp -i assets/fonts/CGA8x8thin.png -o fonts_debug_font > baked/cga_thin.c
$BAKE font -i assets/fonts/bismuth.png -o fonts_bismuth_font > baked/bismuth.c
$BAKE 4bpp -i assets/map/tile_highlight.png -o map_tile_highlight > baked/tile_highlight.c
$BAKE 4bpp -i assets/map/tile_cursor.png -o map_tile_cursor > baked/tile_cursor.c
$BAKE 4bpp -i assets/map/tile_pointer.png -o map_tile_pointer > baked/tile_pointer.c
$BAKE 4bpp -s $CHARACTER -i assets/characters/bjin.png -o characters_bjin > baked/char_bjin.c
$BAKE map -i assets/map/demo/map -t assets/map/demo/tileset.png -o map_demo_map > baked/demo_tileset.c
$BAKE bg -i assets/interface/speech_bubble.png -o interface_speech_bubble > baked/speech_bubble.c
$BAKE 8bpp -i assets/portraits/Bjin.png -s $PORTRAIT --offset=6 -o portraits_Bjin > baked/portrait_bjin.c
