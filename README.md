# Chickpea

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0) [![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-sa/4.0/)

A multi-platform game written in C99, with build time asset utilities in Rust.

# Licenses

All code is licensed under [GPL-3.0](https://www.gnu.org/licenses/gpl-3.0.en.html) (see [LICENSE]()), with the following exceptions:

* [src/crt0.s]() and [src/linker.ld]() were taken from the rust-console [gba crate](https://github.com/rust-console/gba) which is under the Apache-2.0 License. Any modifications made to these two files are likewise under the same license.
* [.clang-format]() was lifted from Linux, it and any modifications to it are licensed under [GPL-2.0](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html).

All non-code assets in the [assets/]() directory are licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (see [assets/LICENSE]()), unless they have their own LICENSE file.
