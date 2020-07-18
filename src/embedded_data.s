    .global _default_debug_font
    .global default_debug_font
_default_debug_font:
default_debug_font:
    .align 4
    .incbin "../baked/fonts/debug_font"

    .global _tile_highlight_gfx
    .global tile_highlight_gfx
_tile_highlight_gfx:
tile_highlight_gfx:
    .align 4
    .incbin "../baked/map/tile_highlight"
