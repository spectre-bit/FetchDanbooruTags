it fetches the danbooru tags and the count of posts tagged with this tag in the following format
```
tag,count
```
One tag-count pair per line.
You can specify a minimum amount of posts in danbooru that have to be tagged with a tag to be added to the output file.

You can download a prebuild exe at the releases tab.

# Usage
Arguments written in <> are optional
```
fetchDanbooruTags.exe <minimum post count>
```

# Build (on Windows using clang++ and ninja)
## Requirements
to be able to build this repository following this tutorial you have to have the following programs installed:
- llvm
- cmake
- vcpkg
- ninja
- msvc toolchain

Also llvm needs to use the msvc toolchain, llvm with mingw does not work with the command i give you here.
But you can also build it with other toolchains but then you need to write the cmake command by yourself.

## Clone the repository
clone the repository into a directory of your choice and change your current directory to the repository
```
git clone https://github.com/spectre-bit/FetchDanbooruTags.git
cd FetchDanbooruTags
```

## build
for the cmake command you need to copy the path to your vcpkg folder and replace <path to vcpkg> with the actual path in the -DCMAKE_TOOLCHAIN_FILE argument
```
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_TOOLCHAIN_FILE="<path to vcpkg>/scripts/buildsystems/vcpkg.cmake" -DCMAKE_CXX_FLAGS="-O3 -march=native -flto -DNDEBUG" -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld-link -flto"
```
```
ninja -C build
```
the compiled exe will be in the created "build" folder

you can also compile this using other compilers and other build systems and other operating systems but then you have to figure out how to do so by yourself
