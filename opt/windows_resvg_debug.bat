git clone https://github.com/RazrFalcon/resvg
cd resvg\crates\c-api
cargo build
cd ..\..
md ..\..\lib64
md ..\..\bin64
copy target\debug\resvg.dll.lib ..\..\lib64\resvgd.lib
copy target\debug\resvg.dll ..\..\bin64
copy /y crates\c-api\resvg.h ..\..\include
cd ..
