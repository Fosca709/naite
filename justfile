configure:
    cmake -S . -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="{{justfile_directory()}}/dist"

build: configure
    cmake --build build/release --config Release
    cmake --install build/release --config Release

package: configure
    cmake --build build/release --config Release
    cpack --config build/release/CPackConfig.cmake -G DEB -B build/packages
