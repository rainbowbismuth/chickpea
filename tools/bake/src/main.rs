use std::collections::HashMap;
use std::io;
use std::path::Path;

use clap::Clap;
use png::{BitDepth, ColorType};

use lz77::encode_gba_lz77;

mod lz77;

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
            self.0[7], self.0[6], self.0[5], self.0[4], self.0[3], self.0[2],
            self.0[1], self.0[0],
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
/// An 8x8 character, with eight bits per pixel.
pub struct Character8BPP(pub [u64; 8]);

impl Character8BPP {
    pub const fn new(data: [u64; 8]) -> Self {
        Self(data)
    }

    pub const fn blank() -> Self {
        Self([0; 8])
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
pub struct Image4BPP {
    pub width: u32,
    pub height: u32,
    pub palette: Palette,
    pub tiles: Vec<Character4BPP>,
}

#[derive(Clone)]
pub struct Image8BPP {
    pub width: u32,
    pub height: u32,
    pub palettes: Vec<Palette>,
    pub tiles: Vec<Character8BPP>,
}

#[derive(Copy, Clone, PartialEq, Eq)]
pub struct Vec2 {
    x: i16,
    y: i16,
}

const X_OFFSET: i16 = 15;

impl Vec2 {
    pub fn new(x: i16, y: i16) -> Self {
        Self { x, y }
    }

    pub fn map_to_tile(self, height: i16) -> Self {
        Self {
            x: X_OFFSET + 2 * (self.x - self.y),
            y: self.x + self.y - height,
        }
    }

    pub fn tile_to_map(self, height: i16) -> Self {
        Self {
            x: (self.x + 2 * self.y + 2 * height - X_OFFSET) / 4,
            y: (-self.x + 2 * self.y + 2 * height + X_OFFSET) / 4,
        }
    }
}

impl Image4BPP {
    pub fn load_png<P: AsRef<Path>>(path: P) -> Self {
        Self::load_png_impl(path.as_ref())
    }

    fn load_png_impl(path: &Path) -> Self {
        let data = std::fs::File::open(path).expect("couldn't find image");
        let mut decoder = png::Decoder::new(data);
        decoder.set_transformations(png::Transformations::IDENTITY);
        let (output_info, mut reader) = decoder.read_info().unwrap();

        let info = reader.info();
        assert_eq!(info.color_type, ColorType::Indexed);
        assert_eq!(info.bit_depth, BitDepth::Eight);
        let png_palette =
            info.palette.to_owned().expect("image must have a palette");
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
                            ((png_pixels[img_x + img_y * pitch] & 0xF) as u32)
                                << (j * 4);
                    }
                }
                tiles.push(gfx);
            }
        }

        Self {
            width,
            height,
            palette,
            tiles,
        }
    }
}

impl Image8BPP {
    pub fn load_png<P: AsRef<Path>>(path: P, offset: usize) -> Self {
        Self::load_png_impl(path.as_ref(), offset)
    }

    fn load_png_impl(path: &Path, offset: usize) -> Self {
        let data = std::fs::File::open(path).expect("couldn't find image");
        let mut decoder = png::Decoder::new(data);
        decoder.set_transformations(png::Transformations::IDENTITY);
        let (output_info, mut reader) = decoder.read_info().unwrap();

        let info = reader.info();
        assert_eq!(info.color_type, ColorType::Indexed);
        assert_eq!(info.bit_depth, BitDepth::Eight);
        let png_palette =
            info.palette.to_owned().expect("image must have a palette");
        let width = info.width;
        let height = info.height;
        assert_eq!(width % 8, 0);
        assert_eq!(height % 8, 0);
        let mut png_pixels = vec![0; output_info.buffer_size()];
        reader.next_frame(&mut png_pixels).unwrap();

        let mut palettes = vec![Palette::ALL_BLACK];
        let colors = png_palette.len() / 3;
        assert!(colors <= 256 - offset * 16);

        for i in 0..colors {
            if i != 0 && i % 16 == 0 {
                palettes.push(Palette::ALL_BLACK);
            }
            let r = png_palette[i * 3 + 0];
            let g = png_palette[i * 3 + 1];
            let b = png_palette[i * 3 + 2];
            let color = rgb8(r, g, b);
            palettes[i / 16].0[i % 16] = color;
        }

        let mut tiles = vec![];
        let pitch = width as usize;
        let tiles_y = height / 8;
        let tiles_x = width / 8;
        for y in 0..tiles_y {
            let y_base = (y * 8) as usize;
            for x in 0..tiles_x {
                let x_base = (x * 8) as usize;
                let mut gfx = Character8BPP::blank();
                for i in 0..8 {
                    for j in 0..8 {
                        let img_x = x_base + j as usize;
                        let img_y = y_base + i as usize;
                        if png_pixels[img_x + img_y * pitch] == 0 {
                            continue;
                        }
                        gfx.0[i as usize] |=
                            (((png_pixels[img_x + img_y * pitch]
                                + offset as u8 * 16)
                                & 0xFF) as u64)
                                << (j * 8);
                    }
                }
                tiles.push(gfx);
            }
        }

        Self {
            width,
            height,
            palettes,
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

impl Serialize for u64 {
    fn write(&self, out: &mut Vec<u8>) {
        out.push(*self as u8);
        out.push((*self >> 8) as u8);
        out.push((*self >> 16) as u8);
        out.push((*self >> 24) as u8);
        out.push((*self >> 32) as u8);
        out.push((*self >> 40) as u8);
        out.push((*self >> 48) as u8);
        out.push((*self >> 56) as u8);
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

impl Serialize for Character8BPP {
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

    #[clap(name = "8bpp")]
    Bake8BPP(Bake8BPP),

    #[clap(name = "map")]
    BakeTileMap(BakeTileMap),

    #[clap(name = "font")]
    BakeFont(BakeFont),

    #[clap(name = "bg")]
    BakeBackground(BakeBackground),
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
struct Bake8BPP {
    #[clap(short = 'i')]
    input: String,

    #[clap(short = 's')]
    swizzle: Option<String>,

    #[clap(short = 'o')]
    output: String,

    #[clap(long = "offset")]
    offset: usize,
}

#[derive(Clap)]
struct BakeTileMap {
    #[clap(short = 'i')]
    input: String,

    #[clap(short = 't')]
    tile_set: String,

    #[clap(short = 'o')]
    output: String,
}

#[derive(Clap)]
struct BakeFont {
    #[clap(short = 'i')]
    input: String,

    #[clap(short = 'o')]
    output: String,
}

#[derive(Clap)]
struct BakeBackground {
    #[clap(short = 'i')]
    input: String,

    #[clap(short = 'o')]
    output: String,
}

fn c_to_stdout(name: &str, data: &[u8]) {
    let uncompressed_len = data.len();

    let compressed = encode_gba_lz77(data);
    let mut lz77_compressed = false;
    let out_data = if compressed.len() < uncompressed_len {
        lz77_compressed = true;
        &compressed
    } else {
        data
    };

    println!(
        "static const uint8_t {}_bytes[] __attribute__((aligned(4))) = {{",
        name
    );
    for chunk in out_data.chunks(12) {
        print!("\t");
        for ch in chunk {
            print!("0x{:02X}, ", ch);
        }
        println!();
    }
    println!("}};");
    println!();
    println!("struct resource {} = {{", name);
    println!("\t.length = {},", uncompressed_len);
    println!("\t.lz77 = {},", lz77_compressed);
    println!("\t.data = {}_bytes,", name);
    println!("}};");
    println!();
}

fn swizzle_to_pattern(swizzle: String) -> Vec<usize> {
    let pattern: Vec<usize> = swizzle
        .chars()
        .filter(|c| c.is_alphanumeric())
        .map(|c| c.to_digit(16).unwrap() as usize)
        .collect();
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

fn bake_4bpp(args: Bake4BPP) -> io::Result<Vec<Character4BPP>> {
    let img = Image4BPP::load_png(&args.input);

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

    c_to_stdout(
        &(args.output.clone() + "_4bpp"),
        &serialize(out_tiles.as_slice()),
    );
    c_to_stdout(&(args.output.clone() + "_pal"), &serialize(&img.palette));

    Ok(out_tiles)
}

fn bake_8bpp(args: Bake8BPP) -> io::Result<()> {
    let img = Image8BPP::load_png(&args.input, args.offset);

    let out_tiles = if let Some(swizzle) = args.swizzle {
        let pattern = swizzle_to_pattern(swizzle);

        let mut swizzled_tiles = vec![Character8BPP::blank(); img.tiles.len()];
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

    c_to_stdout(
        &(args.output.clone() + "_8bpp"),
        &serialize(out_tiles.as_slice()),
    );
    c_to_stdout(
        &(args.output.clone() + "_pals"),
        &serialize(img.palettes.as_slice()),
    );

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
        self.mapping.insert(
            character.flip_horizontal(),
            Tile {
                name: idx,
                horizontal_flip: true,
                vertical_flip: false,
            },
        );
        self.mapping.insert(
            character.flip_vertical(),
            Tile {
                name: idx,
                horizontal_flip: false,
                vertical_flip: true,
            },
        );
        self.mapping.insert(
            character.flip_both(),
            Tile {
                name: idx,
                horizontal_flip: true,
                vertical_flip: true,
            },
        );
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

fn tile_map_with_tile_set(tile_set: &TileSet, map: &[isize]) -> Vec<u16> {
    let mut out = vec![];
    for index in map {
        let tile = tile_set.get(*index).unwrap();
        assert!(tile.name < 1024);
        let val = (tile.name as u16)
            | (tile.horizontal_flip as u16) << 10
            | (tile.vertical_flip as u16) << 11;
        out.push(val);
    }
    out
}

fn compute_height_map(map: &[isize]) -> Vec<u8> {
    let mut out = vec![0; 32 * 32];
    for (i, tile) in map.iter().enumerate() {
        if tile < &0 {
            continue;
        }
        let x = (i % 32) as i16;
        let y = (i / 32) as i16;
        let height = (tile - '0' as isize) as i16;
        let loc = Vec2::new(x - 1, y).tile_to_map(height);
        out[(loc.x + loc.y * 32) as usize] = height as u8;
    }
    out
}

pub const WALK: u8 = 1 << 0;
pub const OCCLUDE_BOTTOM_LEFT: u8 = 1 << 1;
pub const OCCLUDE_BOTTOM_RIGHT: u8 = 1 << 2;

pub const BOTTOM_ARROW_TILE: isize = 25;
pub const RIGHT_ARROW_TILE: isize = 26;
pub const LEFT_ARROW_TILE: isize = 27;

fn compute_attribute_map(
    walk: &[isize],
    occlude: &[isize],
    height_map: &[isize],
) -> Vec<u8> {
    let mut out = vec![0u8; 32 * 32];
    for (i, height_tile) in height_map.iter().enumerate() {
        if height_tile < &0 {
            continue;
        }
        let mut val = 0;

        if walk[i] > 0 {
            val |= WALK;
        }

        match occlude[i] {
            BOTTOM_ARROW_TILE => {
                val |= OCCLUDE_BOTTOM_LEFT | OCCLUDE_BOTTOM_RIGHT;
            }
            RIGHT_ARROW_TILE => {
                val |= OCCLUDE_BOTTOM_RIGHT;
            }
            LEFT_ARROW_TILE => {
                val |= OCCLUDE_BOTTOM_LEFT;
            }
            _ => {}
        };

        let height = (height_map[i] - ('0' as isize)) as i16;
        let x = (i % 32) as i16;
        let y = (i / 32) as i16;
        let loc = Vec2::new(x - 1, y).tile_to_map(height);
        out[(loc.x + loc.y * 32) as usize] = val;
    }
    out
}

fn bake_tilemap(args: BakeTileMap) -> io::Result<()> {
    let img = Image4BPP::load_png(&args.tile_set);
    let mut tile_set = TileSet::new();
    for (i, character) in img.tiles.iter().enumerate() {
        tile_set.add(i as isize, character);
    }

    c_to_stdout(
        &(args.output.clone() + "_4bpp"),
        &serialize(tile_set.tiles.as_slice()),
    );
    c_to_stdout(&(args.output.clone() + "_pal"), &serialize(&img.palette));

    let tile_map_low = parse_tile_map_csv(std::fs::read_to_string(
        &(args.input.clone() + "_low.csv"),
    )?);

    let tile_map_high = parse_tile_map_csv(std::fs::read_to_string(
        &(args.input.clone() + "_high.csv"),
    )?);

    let low_tiles = tile_map_with_tile_set(&tile_set, &tile_map_low);
    let high_tiles = tile_map_with_tile_set(&tile_set, &tile_map_high);

    c_to_stdout(
        &(args.output.clone() + "_low"),
        &serialize(low_tiles.as_slice()),
    );
    c_to_stdout(
        &(args.output.clone() + "_high"),
        &serialize(high_tiles.as_slice()),
    );

    let tile_map_height = parse_tile_map_csv(std::fs::read_to_string(
        &(args.input.clone() + "_height.csv"),
    )?);
    let height_map = compute_height_map(&tile_map_height);

    c_to_stdout(
        &(args.output.clone() + "_height"),
        &serialize(height_map.as_slice()),
    );

    let tile_map_walk = parse_tile_map_csv(std::fs::read_to_string(
        &(args.input.clone() + "_walkable.csv"),
    )?);

    let tile_map_occlude = parse_tile_map_csv(std::fs::read_to_string(
        &(args.input.clone() + "_occlusion.csv"),
    )?);

    let attr_map = compute_attribute_map(
        &tile_map_walk,
        &tile_map_occlude,
        &tile_map_height,
    );

    c_to_stdout(
        &(args.output.clone() + "_attributes"),
        &serialize(attr_map.as_slice()),
    );

    Ok(())
}

pub const FONT_8X16_SWIZZLE: &str = "0123456789ABCDEF0123456789ABCDEF";

fn width_of_char(char: &Character4BPP) -> u8 {
    // TODO: This disturbs me, did I mess up endian-ness or something?
    //  re: leading_zeros() not trailing_zeros() ???
    char.0
        .iter()
        .map(|l| 8 - (l.leading_zeros() / 4))
        .max()
        .unwrap() as u8
}

fn bake_font(args: BakeFont) -> io::Result<()> {
    let tiles = bake_4bpp(Bake4BPP {
        input: args.input.clone(),
        swizzle: Some(String::from(FONT_8X16_SWIZZLE)),
        output: args.output.clone(),
    })?;

    let mut widths: Vec<u8> = Vec::with_capacity(tiles.len() / 2);

    for i in 0..tiles.len() / 2 {
        let width =
            width_of_char(&tiles[i * 2]).max(width_of_char(&tiles[i * 2 + 1]));
        widths.push(width);
    }

    c_to_stdout(
        &(args.output.clone() + "_width"),
        &serialize(widths.as_slice()),
    );

    Ok(())
}

fn bake_background(args: BakeBackground) -> io::Result<()> {
    let img = Image4BPP::load_png(&args.input);
    let mut tile_set = TileSet::new();
    for (i, character) in img.tiles.iter().enumerate() {
        tile_set.add(i as isize, character);
    }

    let mut map = Vec::with_capacity(img.tiles.len());
    for i in 0..img.tiles.len() {
        map.push(i as isize);
    }
    let tile_map = tile_map_with_tile_set(&tile_set, &map);

    c_to_stdout(
        &(args.output.clone() + "_4bpp"),
        &serialize(tile_set.tiles.as_slice()),
    );
    c_to_stdout(&(args.output.clone() + "_pal"), &serialize(&img.palette));
    c_to_stdout(
        &(args.output.clone() + "_tiles"),
        &serialize(tile_map.as_slice()),
    );

    Ok(())
}

fn main() -> io::Result<()> {
    let opts: Opts = Opts::parse();

    println!("#include \"game/resource.h\"\n");
    match opts.sub_cmd {
        SubCommand::Bake4BPP(args) => bake_4bpp(args).map(|_x| ()),
        SubCommand::Bake8BPP(args) => bake_8bpp(args),
        SubCommand::BakeTileMap(args) => bake_tilemap(args),
        SubCommand::BakeFont(args) => bake_font(args),
        SubCommand::BakeBackground(args) => bake_background(args),
    }
}
