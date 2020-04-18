rm -rf build
mkdir -p build
cd  build

rm -rf *

mkdir -p www.dswd.net
cp -r ../www.dswd.net/* ./www.dswd.net/
cp  ../*.xml  ./
mkdir -p web 
cp -r ../web/*   ./web
mkdir -p dict 
cp -r ../jieba/dict/*  ./dict/
cp -r ../*.utf8  ./dict/
cp   ../monitorKeywordSvr.sh  ./
cp -r ../*.php  ./
cp  ../*.crt  ./
cp  ../*.key  ./
chmod +777  *.php
cmake ..
make




