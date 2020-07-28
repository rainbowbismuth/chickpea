# chickpea-c

A multi-platform game written in C99, with build time asset utilities in Rust.

# License

All code is licensed under GPL-3.0, with the following exceptions:

* [src/crt0.s]() and [src/linker.ld]() were taken from the rust-console 
[gba crate](https://github.com/rust-console/gba) which is under the Apache-2.0
License. Any modifications made to these two files are likewise under the same 
license.

* [.clang-format]() was lifted from Linux, and as described in the file, it and
any modifications are licensed under GPL-2.0.