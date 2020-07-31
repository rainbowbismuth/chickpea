.macro include_bin, name:req, path:req
    .global \name
    .global _\name
\name:
_\name:
    .align 4
    .incbin "\path"
.endm

    include_bin debug_font_4bpp, "../baked/fonts/debug_font.4bpp"
    include_bin debug_font_pal, "../baked/fonts/debug_font.pal"
    include_bin tile_highlight_4bpp, "../baked/map/tile_highlight.4bpp"
    include_bin tile_highlight_pal, "../baked/map/tile_highlight.pal"
    include_bin tile_cursor_4bpp, "../baked/map/tile_cursor.4bpp"
    include_bin tile_cursor_pal, "../baked/map/tile_cursor.pal"
    include_bin soldier_4bpp, "../baked/characters/soldier.4bpp"
    include_bin soldier_pal, "../baked/characters/soldier.pal"

    include_bin demo_map_4bpp, "../baked/map/demo/map.4bpp"
    include_bin demo_map_pal, "../baked/map/demo/map.pal"
    include_bin demo_map_low, "../baked/map/demo/map.low"
    include_bin demo_map_high, "../baked/map/demo/map.high"
    include_bin demo_map_height, "../baked/map/demo/map.height"
    include_bin demo_map_attributes, "../baked/map/demo/map.attributes"