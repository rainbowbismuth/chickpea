#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

BAKE=tools/bake/target/debug/bake

$BAKE 4bpp -i assets/map/tile_highlight.png -o baked/map/tile_highlight
$BAKE 4bpp -i assets/map/tile_cursor.png -o baked/map/tile_cursor
$BAKE 4bpp -i assets/map/tile_pointer.png -o baked/map/tile_pointer
$BAKE 4bpp -s "01012323" -i assets/characters/soldier.png -o baked/characters/soldier
$BAKE map -i assets/map/demo/map -t assets/map/demo/tileset.png -o baked/map/demo/map
