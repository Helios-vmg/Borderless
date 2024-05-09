#!/bin/sh
git clone https://github.com/RazrFalcon/resvg
cd resvg/crates/c-api
cargo build --release
cd ../..
mkdir ../../lib
cp -f target/release/libresvg.a ../../lib
cp -f crates/c-api/resvg.h ../../include
cd ..
