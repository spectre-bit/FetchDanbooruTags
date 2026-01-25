# Usage
Arguments written in <> are optional
```
fetchDanbooruTags.exe <minimum post count>
```

# Build (on Windows using clang++ and ninja)
## Requirements
to be able to build this repository following this tutorial you have to have the following programs installed:
- clang++
- cmake
- vcpkg
- ninja

## Clone the repository
clone the repository into a directory of your choice and change your current directory to the repository
```
git clone https://github.com/spectre-bit/FetchDanbooruTags.git
cd FetchDanbooruTags
```

## Install dependencies
run the following command
```
vcpkg install
```

## build
for the cmake command you need to copy the path to your vcpkg folder and paste it in the marked place in the -DCMAKE_TOOLCHAIN_FILE argument
```
mkdir build
```
```
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DVCPKG_TARGET_TRIPLETT="x64-windows-clang" -DCMAKE_TOOLCHAIN_FILE="C:<path to vcpkg>/scripts/buildsystems/vcpkg.cmake" -DCMAKE_CXX_FLAGS="-O3 -march=native -flto -DNDEBUG" -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld -Wl,--gc-sections -s" -B build
```
```
ninja -C build
```
the compiled exe will be in the created "build" folder

you can also compile this using other compilers and other build systems and other operating systems but then you have to figure out how to do so by yourself
