#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

BAKE=tools/bake/target/debug/bake

# Tile swizzle patterns
CHARACTER="01012323"

$BAKE 4bpp -i assets/fonts/CGA8x8thin.png -o baked/fonts/debug_font
$BAKE font -i assets/fonts/bismuth.png -o baked/fonts/bismuth_font
$BAKE 4bpp -i assets/map/tile_highlight.png -o baked/map/tile_highlight
$BAKE 4bpp -i assets/map/tile_cursor.png -o baked/map/tile_cursor
$BAKE 4bpp -i assets/map/tile_pointer.png -o baked/map/tile_pointer
$BAKE 4bpp -s $CHARACTER -i assets/characters/soldier.png -o baked/characters/soldier
$BAKE map -i assets/map/demo/map -t assets/map/demo/tileset.png -o baked/map/demo/map
