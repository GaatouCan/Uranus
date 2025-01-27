@echo off

echo "Creating Third Library..."

set SOURCE_DIR=./source
set INSTALL_DIR=D:/library/install

set ASIO_DIR=asio-1.30.2
set LUA_DIR=lua-5.4.7

set ZLIB_INC_DIR=%INSTALL_DIR%/zlib/include
set ZLIB_LIB_DEBUG=%INSTALL_DIR%/zlib/lib/zlibd.lib
set ZLIB_LIB_RELEASE=%INSTALL_DIR%/zlib/lib/zlib.lib

set CMAKE=cmake

set TARGET_LIST=

if NOT exist %SOURCE_DIR% (
    mkdir %SOURCE_DIR%
)

if NOT exist %INSTALL_DIR% (
    mkdir %INSTALL_DIR%
)

cd %SOURCE_DIR%

@REM spdlog
@REM git clone -b v1.15.0 https://github.com/gabime/spdlog.git

cd ./spdlog
call %CMAKE% . -G "Visual Studio 17 2022" -B ./build -DCMAKE_DEBUG_POSTFIX='d' -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR%/spdlog -DSPDLOG_BUILD_EXAMPLE=OFF -DSPDLOG_BUILD_SHARED=ON

set TARGET_LIST=%TARGET_LIST% spdlog
cd ../

@REM asio
if exist %ASIO_DIR% (
    cd %ASIO_DIR%
    call %CMAKE% . -G "Visual Studio 17 2022" -B ./build -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR%/asio
    
    set TARGET_LIST=%TARGET_LIST% %ASIO_DIR%
    cd ../
)


@REM lua
if exist %LUA_DIR% (
    cd %LUA_DIR%
    call %CMAKE% . -G "Visual Studio 17 2022" -B ./build -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR%/lua
    
    set TARGET_LIST=%TARGET_LIST% %LUA_DIR%
    cd ../
)

@REM json
@REM git clone -b v3.11.3 https://github.com/nlohmann/json.git

cd ./json
call %CMAKE% . -G "Visual Studio 17 2022" -B ./build -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR%/nlohmann_json -DJSON_BuildTests=OFF -DBUILD_TESTING=OFF

set TARGET_LIST=%TARGET_LIST% json
cd ../

@REM YAML
@REM git clone -b 0.8.0 https://github.com/jbeder/yaml-cpp.git

cd ./yaml-cpp
call %CMAKE% . -G "Visual Studio 17 2022" -B ./build -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR%/YAML_CPP -DYAML_BUILD_SHARED_LIBS=ON -DYAML_CPP_BUILD_TESTS=OFF -DBUILD_TESTING=OFF

set TARGET_LIST=%TARGET_LIST% yaml-cpp
cd ../

@REM zlib
@REM git clone -b v1.3.1 https://github.com/madler/zlib.git

cd ./zlib
call %CMAKE% . -G "Visual Studio 17 2022" -B ./build -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR%/zlib -DCMAKE_DEBUG_POSTFIX='d' -DINSTALL_BIN_DIR=%INSTALL_DIR%/zlib/bin -DINSTALL_INC_DIR=%INSTALL_DIR%/zlib/include -DINSTALL_LIB_DIR=%INSTALL_DIR%/zlib/lib -DINSTALL_MAN_DIR=%INSTALL_DIR%/zlib/share/man -DINSTALL_PKGCONFIG_DIR=%INSTALL_DIR%/zlib/share/pkgconfig

set TARGET_LIST=%TARGET_LIST% zlib
cd ../

for %%i in (%TARGET_LIST%) do (
    cd %%i/build
    cd

    call %CMAKE% --build . --target install --config=Debug
    call %CMAKE% --build . --target install --config=Release

    cd ../../
)

@REM protobuf
@REM git clone -b v29.1 https://github.com/protocolbuffers/protobuf.git

cd ./protobuf
git submodule update --init --recursive

call %CMAKE% . -G "Visual Studio 17 2022" -B ./build -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR%/protobuf -DCMAKE_DEBUG_POSTFIX='d' -DCMAKE_CXX_STANDARD=20 -DBUILD_TESTING=OFF -DABSL_PROPAGATE_CXX_STD=ON -Dprotobuf_BUILD_LIBPROTOC=ON -Dprotobuf_BUILD_SHARED_LIBS=ON -Dprotobuf_BUILD_TESTS=OFF -DZLIB_INCLUDE_DIR=%ZLIB_INC_DIR% -DZLIB_LIBRARY_DEBUG=%ZLIB_LIB_DEBUG% -DZLIB_LIBRARY_RELEASE=%ZLIB_LIB_RELEASE%

cd ./build
call %CMAKE% --build . --target install --config=Debug
call %CMAKE% --build . --target install --config=Release

cd ../../

cd ../

pause