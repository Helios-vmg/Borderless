#!/bin/sh
cd serialization
chmod +x build.sh
./build.sh
mv LAS.out ../LAS
cd ../src/serialization
../../LAS settings.txt
