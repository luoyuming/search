mkdir -p build
cd  build

make clean
rm -rf CMakeCache.tx
rm -rf CMakeFiles
rm -rf cmake_install.cmake
rm -rf Makefile

cmake ..
make


