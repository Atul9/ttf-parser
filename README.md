## ttf-parser
[![Build Status](https://travis-ci.org/RazrFalcon/ttf-parser.svg?branch=master)](https://travis-ci.org/RazrFalcon/ttf-parser)
[![Crates.io](https://img.shields.io/crates/v/ttf-parser.svg)](https://crates.io/crates/ttf-parser)
[![Documentation](https://docs.rs/ttf-parser/badge.svg)](https://docs.rs/ttf-parser)
[![Rust 1.35+](https://img.shields.io/badge/rust-1.35+-orange.svg)](https://www.rust-lang.org)
![](https://img.shields.io/badge/unsafe-forbidden-brightgreen.svg)

A high-level, safe, zero-allocation TrueType font parser.

Can be used as Rust and as C library.

### Features

- A high-level API, for people who doesn't know how TrueType works internally.
  Basically, no direct access to font tables.
- A [C API](./c-api).
- Zero heap allocations.
- Zero unsafe.
- Zero required dependencies. Logging is enabled by default.
- `no_std` compatible.
- Fast. Set the *Performance* section.
- Stateless. No mutable methods.
- Simple and maintainable code (no magic numbers).

### Safety

- The library must not panic. Any panic considered as a critical bug and should be reported.
- The library forbids the unsafe code.
- No heap allocations, so crash due to OOM is not possible.
- All recursive methods have a depth limit.
- Technically, should use less than 64KiB of stack in worst case scenario.
- Most of arithmetic operations are checked.

### Alternatives

It's very hard to compare different libraries, so we are using table-based comparison.
There are roughly three types of TrueType tables:

- A table with a list of properties (like `head`, `OS/2`, etc.).<br/>
  If a library tries to parse it at all then we mark it as supported.
- A table that contains a single type of data (`glyf`, `CFF` (kinda), `hmtx`, etc.).<br/>
  Can only be supported or not.
- A table that contains multiple subtables (`cmap`, `kern`, `GPOS`, etc.).<br/>
  Can be partially supported and we note which subtables are actually supported.

| Feature/Library   | ttf-parser             | FreeType            | stb_truetype                   |
| ----------------- | :--------------------: | :-----------------: | :----------------------------: |
| Memory safe       | ✓                      |                     |                                |
| Thread safe       | ✓                      |                     |                                |
| Zero allocation   | ✓                      |                     |                                |
| Variable fonts    | ✓                      | ✓                   |                                |
| `avar` table      | ✓                      | ✓                   |                                |
| `CFF `&nbsp;table | ✓                      | ✓                   | ✓                              |
| `CFF2` table      | ✓                      | ✓                   |                                |
| `cmap` table      | ~ (no 8; Unicode-only) | ✓                   | ~ (no 2,8,10,14; Unicode-only) |
| `fvar` table      | ✓                      | ✓                   |                                |
| `gasp` table      |                        | ✓                   |                                |
| `GDEF` table      | ~                      |                     |                                |
| `glyf` table      | ✓                      | ✓                   | ✓                              |
| `GPOS` table      |                        |                     | ~ (only 2)                     |
| `GSUB` table      |                        |                     |                                |
| `gvar` table      | ✓                      | ✓                   |                                |
| `head` table      | ✓                      | ✓                   | ✓                              |
| `hhea` table      | ✓                      | ✓                   | ✓                              |
| `hmtx` table      | ✓                      | ✓                   | ✓                              |
| `HVAR` table      | ✓                      | ✓                   |                                |
| `kern` table      | ~                      | ~                   | ~                              |
| `maxp` table      | ✓                      | ✓                   | ✓                              |
| `MVAR` table      | ✓                      | ✓                   |                                |
| `name` table      | ✓                      | ✓                   |                                |
| `OS/2` table      | ✓                      | ✓                   |                                |
| `post` table      | ✓                      | ✓                   |                                |
| `SVG `&nbsp;table |                        |                     | ✓                              |
| `vhea` table      | ✓                      | ✓                   |                                |
| `vmtx` table      | ✓                      | ✓                   |                                |
| `VORG` table      | ✓                      | ✓                   |                                |
| `VVAR` table      | ✓                      | ✓                   |                                |
| Rendering         |                        | ✓                   | ~<sup>1</sup>                  |
| Language          | Rust + C API           | C                   | C                              |
| Dynamic lib size  | ~250KiB                | ~760KiB<sup>2</sup> | ? (header-only)                |
| Tested version    | 0.4.0                  | 2.9.1               | 1.24                           |
| License           | MIT / Apache-2.0       | FTL/GPLv2           | public domain                  |

Legend:

- ✓ - supported
- ~ - partial
- *nothing* - not supported

Notes:

1. Very primitive.
2. Depends on build flags.

### Performance

TrueType fonts designed for fast querying, so most of the methods are very fast.
The main exception is glyph outlining. Glyphs can be stored using two different methods:
using [Glyph Data](https://docs.microsoft.com/en-us/typography/opentype/spec/glyf) format
and [Compact Font Format](http://wwwimages.adobe.com/content/dam/Adobe/en/devnet/font/pdfs/5176.CFF.pdf) (pdf).
The first one is fairly simple which makes it faster to process.
The second one is basically a tiny language with a stack-based VM, which makes it way harder to process.

The [benchmark](./benches/outline/) tests how long it takes to outline all glyphs in the font.

| Table/Library | ttf-parser         | FreeType       | stb_truetype     |
| ------------- | -----------------: | -------------: | ---------------: |
| `glyf`        |     `786'180 ns`   | `1'194'395 ns` | **`695'873 ns`** |
| `gvar`        | **`3'487'405 ns`** | `3'594'170 ns` |                - |
| `CFF`         | **`1'237'364 ns`** | `5'806'994 ns` |  `2'862'264 ns`  |
| `CFF2`        | **`1'893'664 ns`** | `6'782'960 ns` |                - |

**Note:** FreeType is surprisingly slow, so I'm worried that I've messed something up.

And here are some methods benchmarks:

```text
test outline_glyph_276_from_cff  ... bench:         841 ns/iter (+/- 53)
test outline_glyph_276_from_glyf ... bench:         674 ns/iter (+/- 15)
test from_data_otf_cff           ... bench:         403 ns/iter (+/- 3)
test outline_glyph_8_from_cff    ... bench:         339 ns/iter (+/- 44)
test outline_glyph_8_from_glyf   ... bench:         295 ns/iter (+/- 16)
test glyph_name_276              ... bench:         214 ns/iter (+/- 1)
test from_data_ttf               ... bench:         169 ns/iter (+/- 3)
test family_name                 ... bench:         155 ns/iter (+/- 5)
test glyph_index_u41             ... bench:          16 ns/iter (+/- 0)
test glyph_name_8                ... bench:           1 ns/iter (+/- 0)
test underline_metrics           ... bench:         0.5 ns/iter (+/- 0)
test units_per_em                ... bench:         0.5 ns/iter (+/- 0)
test strikeout_metrics           ... bench:         0.5 ns/iter (+/- 0)
test x_height                    ... bench:         0.4 ns/iter (+/- 0)
test ascender                    ... bench:         0.2 ns/iter (+/- 0)
test hor_advance                 ... bench:         0.2 ns/iter (+/- 0)
test hor_side_bearing            ... bench:         0.2 ns/iter (+/- 0)
test subscript_metrics           ... bench:         0.2 ns/iter (+/- 0)
test width                       ... bench:         0.2 ns/iter (+/- 0)
```

`family_name` is expensive, because it allocates a `String` and the original data
is stored as UTF-16 BE.

`glyph_name_8` is faster than `glyph_name_276`, because for glyph indexes lower than 258
we are using predefined names, so no parsing is involved.

### Error handling

`ttf-parser` is designed to parse well-formed fonts, so it does not have an `Error` enum.
It doesn't mean that it will crash or panic on malformed fonts, only that the
error handling will boil down to `Option::None`. So you will not get a detailed cause of an error.
By doing so we can simplify an API quite a lot since otherwise, we will have to use
`Result<Option<T>, Error>`.

Some methods may print warnings, when the `logging` feature is enabled.

### License

Licensed under either of

- Apache License, Version 2.0
  ([LICENSE-APACHE](LICENSE-APACHE) or http://www.apache.org/licenses/LICENSE-2.0)
- MIT license
  ([LICENSE-MIT](LICENSE-MIT) or http://opensource.org/licenses/MIT)

at your option.

### Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted
for inclusion in the work by you, as defined in the Apache-2.0 license, shall be
dual licensed as above, without any additional terms or conditions.
