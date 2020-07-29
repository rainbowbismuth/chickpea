#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

BAKE=tools/bake/target/debug/bake

$BAKE 4bpp -i assets/map/tile_highlight.png -o baked/map/tile_highlight
$BAKE 4bpp -s "20032113" -i assets/map/tile_cursor.png -o baked/map/tile_cursor
$BAKE 4bpp -s "01012323" -i assets/characters/soldier.png -o baked/characters/soldier
$BAKE map -t assets/map/demo/tileset.png -l assets/map/demo/map_low.csv -h assets/map/demo/map_high.csv -o baked/map/demo/map
