git clone https://github.com/RazrFalcon/resvg
cd resvg\crates\c-api
cargo build --release
cd ..\..
md ..\..\lib64
copy /y target\release\resvg.lib ..\..\lib64
copy /y crates\c-api\resvg.h ..\..\include
cd ..
