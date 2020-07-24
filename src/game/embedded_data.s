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
