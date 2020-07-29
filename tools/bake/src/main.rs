use std::collections::HashMap;
use std::io;
use std::path::{Path, PathBuf};

use clap::Clap;
use png::{BitDepth, ColorType};

pub fn reverse_nibbles(mut n: u32) -> u32 {
    n = (n & 0x0F0F0F0F) << 4 | (n & 0xF0F0F0F0) >> 4;
    n = (n & 0x00FF00FF) << 8 | (n & 0xFF00FF00) >> 8;
    n = (n & 0x0000FFFF) << 16 | (n & 0xFFFF0000) >> 16;
    n
}

#[derive(Copy, Clone, PartialEq, Eq, Hash)]
#[repr(transparent)]
pub struct Color(pub u16);

impl Color {
    pub const BLACK: Self = Self::new(0, 0, 0);
    pub const WHITE: Self = Self::new(31, 31, 31);

    pub const fn new(r: u8, g: u8, b: u8) -> Self {
        Self(((b as u16) << 10) | ((g as u16) << 5) | (r as u16))
    }

    pub const fn blue(self) -> u8 {
        ((self.0 >> 10) & 0b1_1111) as u8
    }

    pub const fn green(self) -> u8 {
        ((self.0 >> 5) & 0b1_1111) as u8
    }

    pub const fn red(self) -> u8 {
        (self.0 & 0b1_1111) as u8
    }
}

pub const fn rgb(r: u8, g: u8, b: u8) -> Color {
    Color::new(r, g, b)
}

pub const fn rgb8(r: u8, g: u8, b: u8) -> Color {
    Color::new(r >> 3, g >> 3, b >> 3)
}


#[derive(Copy, Clone, PartialEq, Eq, Hash)]
#[repr(transparent)]
/// An 8x8 character, with four bits per pixel. Each nibble indexes into a palette.
pub struct Character4BPP(pub [u32; 8]);

impl Character4BPP {
    pub const fn new(data: [u32; 8]) -> Self {
        Self(data)
    }

    pub const fn blank() -> Self {
        Self([0; 8])
    }

    pub const fn line(&self, y: usize) -> u32 {
        self.0[y]
    }


    pub fn flip_vertical(&self) -> Self {
        Self([
            self.0[7], self.0[6], self.0[5], self.0[4], self.0[3], self.0[2], self.0[1], self.0[0],
        ])
    }

    pub fn flip_horizontal(&self) -> Self {
        Self([
            reverse_nibbles(self.0[0]),
            reverse_nibbles(self.0[1]),
            reverse_nibbles(self.0[2]),
            reverse_nibbles(self.0[3]),
            reverse_nibbles(self.0[4]),
            reverse_nibbles(self.0[5]),
            reverse_nibbles(self.0[6]),
            reverse_nibbles(self.0[7]),
        ])
    }

    pub fn flip_both(&self) -> Self {
        self.flip_horizontal().flip_vertical()
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Hash)]
#[repr(transparent)]
/// A sixteen color, RGB palette.
pub struct Palette(pub [Color; 16]);

impl Palette {
    pub const ALL_BLACK: Self = Self::new([Color::BLACK; 16]);

    pub const fn new(colors: [Color; 16]) -> Self {
        Self(colors)
    }
}

#[derive(Clone)]
pub struct Image {
    pub width: u32,
    pub height: u32,
    pub palette: Palette,
    pub tiles: Vec<Character4BPP>,
}

impl Image {
    pub fn load_png<P: AsRef<Path>>(path: P) -> Image {
        Self::load_png_impl(path.as_ref())
    }

    fn load_png_impl(path: &Path) -> Image {
        let data = std::fs::File::open(path).expect("couldn't find image");
        let mut decoder = png::Decoder::new(data);
        decoder.set_transformations(png::Transformations::IDENTITY);
        let (output_info, mut reader) = decoder.read_info().unwrap();

        let info = reader.info();
        assert_eq!(info.color_type, ColorType::Indexed);
        assert_eq!(info.bit_depth, BitDepth::Eight);
        let png_palette = info.palette.to_owned().expect("image must have a palette");
        let width = info.width;
        let height = info.height;
        assert_eq!(width % 8, 0);
        assert_eq!(height % 8, 0);
        let mut png_pixels = vec![0; output_info.buffer_size()];
        reader.next_frame(&mut png_pixels).unwrap();

        let mut palette = Palette::ALL_BLACK;

        let colors = png_palette.len() / 3;
        assert!(colors <= 16);

        for i in 0..colors {
            let r = png_palette[i * 3 + 0];
            let g = png_palette[i * 3 + 1];
            let b = png_palette[i * 3 + 2];
            let color = rgb8(r, g, b);
            palette.0[i] = color;
        }

        let mut tiles = vec![];
        let pitch = width as usize;
        let tiles_y = height / 8;
        let tiles_x = width / 8;
        for y in 0..tiles_y {
            let y_base = (y * 8) as usize;

            for x in 0..tiles_x {
                let x_base = (x * 8) as usize;

                let mut gfx = Character4BPP::blank();
                for i in 0..8 {
                    for j in 0..8 {
                        let img_x = x_base + j as usize;
                        let img_y = y_base + i as usize;
                        gfx.0[i as usize] |=
                            ((png_pixels[img_x + img_y * pitch] & 0xF) as u32) << (j * 4);
                    }
                }
                tiles.push(gfx);
            }
        }

        Image {
            width,
            height,
            palette,
            tiles,
        }
    }
}

pub trait Serialize {
    fn write(&self, out: &mut Vec<u8>);
}

impl Serialize for u8 {
    fn write(&self, out: &mut Vec<u8>) {
        out.push(*self);
    }
}

impl Serialize for u16 {
    fn write(&self, out: &mut Vec<u8>) {
        out.push(*self as u8);
        out.push((*self >> 8) as u8);
    }
}

impl Serialize for u32 {
    fn write(&self, out: &mut Vec<u8>) {
        out.push(*self as u8);
        out.push((*self >> 8) as u8);
        out.push((*self >> 16) as u8);
        out.push((*self >> 24) as u8);
    }
}

impl Serialize for Color {
    fn write(&self, out: &mut Vec<u8>) {
        self.0.write(out);
    }
}

impl<T: Serialize> Serialize for [T] {
    fn write(&self, out: &mut Vec<u8>) {
        for value in self {
            value.write(out);
        }
    }
}

impl Serialize for Palette {
    fn write(&self, out: &mut Vec<u8>) {
        self.0.as_ref().write(out);
    }
}

impl Serialize for Character4BPP {
    fn write(&self, out: &mut Vec<u8>) {
        self.0.as_ref().write(out);
    }
}

pub fn serialize<T: Serialize + ?Sized>(val: &T) -> Vec<u8> {
    let mut out = vec![];
    val.write(&mut out);
    out
}

#[derive(Clap)]
#[clap(version = "0.1", author = "Emily A. Bellows")]
struct Opts {
    #[clap(subcommand)]
    sub_cmd: SubCommand,
}

#[derive(Clap)]
enum SubCommand {
    #[clap(name = "4bpp")]
    Bake4BPP(Bake4BPP),

    #[clap(name = "map")]
    BakeTileMap(BakeTileMap),
}

#[derive(Clap)]
struct Bake4BPP {
    #[clap(short = 'i')]
    input: String,

    #[clap(short = 's')]
    swizzle: Option<String>,

    #[clap(short = 'o')]
    output: String,
}

#[derive(Clap)]
struct BakeTileMap {
    #[clap(short = 'h')]
    high_csv: String,

    #[clap(short = 'l')]
    low_csv: String,

    #[clap(short = 't')]
    tile_set: String,

    #[clap(short = 'o')]
    output: String,
}

fn swizzle_to_pattern(swizzle: String) -> Vec<usize> {
    let pattern: Vec<usize> = swizzle.chars().map(|c| c.to_digit(16).unwrap() as usize).collect();
    let mut counts = vec![0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    for idx in &pattern {
        counts[*idx] += 1usize;
    }
    let mut running_offset = 0;
    let mut offsets = Vec::with_capacity(16);
    for count in &counts {
        offsets.push(running_offset);
        running_offset += count;
    }
    let mut counts = vec![0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    let mut index_pattern = Vec::with_capacity(pattern.len());
    for idx in &pattern {
        index_pattern.push(offsets[*idx] + counts[*idx]);
        counts[*idx] += 1;
    }
    index_pattern
}

fn bake_4bpp(args: Bake4BPP) -> io::Result<()> {
    let img = Image::load_png(&args.input);

    let out_tiles = if let Some(swizzle) = args.swizzle {
        let pattern = swizzle_to_pattern(swizzle);

        let mut swizzled_tiles = vec![Character4BPP::blank(); img.tiles.len()];
        let mut chunk_offset = 0;
        for chunk in img.tiles.chunks_exact(pattern.len()) {
            for (tile_i, pattern_i) in pattern.iter().enumerate() {
                swizzled_tiles[chunk_offset + *pattern_i] = chunk[tile_i];
            }
            chunk_offset += pattern.len();
        }
        swizzled_tiles
    } else {
        img.tiles
    };

    let mut out_4bpp = PathBuf::from(&args.output);
    out_4bpp.set_extension("4bpp");
    let mut out_pal = PathBuf::from(&args.output);
    out_pal.set_extension("pal");

    std::fs::write(out_4bpp, serialize(out_tiles.as_slice()))?;
    std::fs::write(out_pal, serialize(&img.palette))?;

    Ok(())
}

#[derive(Copy, Clone)]
struct Tile {
    name: usize,
    horizontal_flip: bool,
    vertical_flip: bool,
    /* TODO: Palettes? */
}

struct TileSet {
    tiles: Vec<Character4BPP>,
    mapping: HashMap<Character4BPP, Tile>,
    original: HashMap<isize, Tile>,
}

impl TileSet {
    pub fn new() -> Self {
        let mut tile_set = Self {
            tiles: vec![],
            mapping: HashMap::new(),
            original: HashMap::new(),
        };
        tile_set.add(-1, &Character4BPP::blank());
        tile_set
    }

    pub fn add(&mut self, original: isize, character: &Character4BPP) {
        if let Some(tile) = self.mapping.get(&character) {
            self.original.insert(original, *tile);
            return;
        }
        let idx = self.tiles.len();
        self.tiles.push(*character);
        let tile = Tile {
            name: idx,
            horizontal_flip: false,
            vertical_flip: false,
        };
        self.mapping.insert(*character, tile);
        self.original.insert(original, tile);
        self.mapping.insert(character.flip_horizontal(), Tile {
            name: idx,
            horizontal_flip: true,
            vertical_flip: false,
        });
        self.mapping.insert(character.flip_vertical(), Tile {
            name: idx,
            horizontal_flip: false,
            vertical_flip: true,
        });
        self.mapping.insert(character.flip_both(), Tile {
            name: idx,
            horizontal_flip: true,
            vertical_flip: true,
        });
    }

    pub fn get(&self, original: isize) -> Option<Tile> {
        self.original.get(&original).copied()
    }
}

fn parse_tile_map_csv(data: String) -> Vec<isize> {
    let mut out = vec![];
    for line in data.split("\n") {
        if line.is_empty() {
            continue;
        }
        for val in line.split(",") {
            let parsed: isize = val.parse().unwrap();
            out.push(parsed);
        }
    }
    out
}

fn tile_map_with_tile_set(tile_set: &TileSet, map: Vec<isize>) -> Vec<u16> {
    let mut out = vec![];
    for index in map {
        let tile = tile_set.get(index).unwrap();
        assert!(tile.name < 1024);
        let val = (tile.name as u16) | (tile.horizontal_flip as u16) << 10 | (tile.vertical_flip as u16) << 11;
        out.push(val);
    }
    out
}

fn bake_tilemap(args: BakeTileMap) -> io::Result<()> {
    let img = Image::load_png(&args.tile_set);
    let mut tile_set = TileSet::new();
    for (i, character) in img.tiles.iter().enumerate() {
        tile_set.add(i as isize, character);
    }

    let mut out_4bpp = PathBuf::from(&args.output);
    out_4bpp.set_extension("4bpp");
    let mut out_pal = PathBuf::from(&args.output);
    out_pal.set_extension("pal");
    std::fs::write(out_4bpp, serialize(tile_set.tiles.as_slice()))?;
    std::fs::write(out_pal, serialize(&img.palette))?;

    let tile_map_low = parse_tile_map_csv(std::fs::read_to_string(&args.low_csv)?);
    let tile_map_high = parse_tile_map_csv(std::fs::read_to_string(&args.high_csv)?);

    let low_tiles = tile_map_with_tile_set(&tile_set, tile_map_low);
    let high_tiles = tile_map_with_tile_set(&tile_set, tile_map_high);

    let mut out_low = PathBuf::from(&args.output);
    out_low.set_extension("low");
    std::fs::write(out_low, serialize(low_tiles.as_slice()))?;

    let mut out_high = PathBuf::from(&args.output);
    out_high.set_extension("high");
    std::fs::write(out_high, serialize(high_tiles.as_slice()))?;

    Ok(())
}

fn main() -> io::Result<()> {
    let opts: Opts = Opts::parse();

    match opts.sub_cmd {
        SubCommand::Bake4BPP(args) => bake_4bpp(args),
        SubCommand::BakeTileMap(args) => bake_tilemap(args)
    }
}
