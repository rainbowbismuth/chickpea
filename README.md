# Chickpea

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0) [![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-sa/4.0/)

A multi-platform game written in C99, with build time asset utilities in Rust.

# Building

* **GBA** - Tell cmake `-DBUILD_GBA=1`. Requires devkitARM, relevant paths are currently hardcoded in [CMakeLists.txt]().
* **Emscripten** - Should build without modification, doesn't build any supporting `.html` file though, just the `.js` and `.wasm` files.
* **MacOS** - Should build without modification, when running cmake you will need to point the cmake prefix path towards the package configs for SDL2, as an example: `-DCMAKE_PREFIX_PATH="/usr/local/Cellar/sdl2/2.0.12_1/lib/cmake/SDL2"`.
* **Linux** - Should build without modification with clang or gcc, requires SDL2 dev package to be installed.

# Licenses

All code is licensed under [GPL-3.0](https://www.gnu.org/licenses/gpl-3.0.en.html) (see [LICENSE]()), with the following exceptions:

* [src/crt0.s]() and [src/linker.ld]() were taken from the rust-console [gba crate](https://github.com/rust-console/gba) which is under the Apache-2.0 License. Any modifications made to these two files are likewise under the same license.
* [.clang-format]() was lifted from Linux, it and any modifications to it are licensed under [GPL-2.0](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html).

All non-code assets in the [assets/]() directory are licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (see [assets/LICENSE]()), unless they have their own LICENSE file.
