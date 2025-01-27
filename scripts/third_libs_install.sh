echo "Creating Third Library..."

SOURCE_DIR="source"
# INSTALL_DIR="../../install"

ASIO_DIR="asio-1.30.2"
#LUA_DIR="lua-5.4.7"

# ZLIB_INC_DIR="$INSTALL_DIR/zlib/include"
# ZLIB_LIB_DEBUG="$INSTALL_DIR/zlib/lib/libz.so"
# ZLIB_LIB_RELEASE="$INSTALL_DIR/zlib/lib/libz.so"

TARGET_LIST=()

# cmake = "F:\MyApps\cmake-3.30.5-windows-x86_64\bin\cmake.exe"

[ -d "$SOURCE_DIR" ] || mkdir -p "$SOURCE_DIR"
#[ -d "$INSTALL_DIR" ] || mkdir -p "$INSTALL_DIR"


cd "$SOURCE_DIR" || exit

# spdlog
# git clone -b v1.15.0 https://github.com/gabime/spdlog.git

cd spdlog || exit
cmake . -G "Unix Makefiles" -B ./build -DSPDLOG_BUILD_EXAMPLE=OFF -DSPDLOG_BUILD_SHARED=ON

TARGET_LIST+=("spdlog")
cd ../

# asio
if [ -d "$ASIO_DIR" ]; then
     cd "$ASIO_DIR" || exit
     cmake . -G "Unix Makefiles" -B ./build

     TARGET_LIST+=("$ASIO_DIR")
     cd ../
fi

# json
# git clone -b v3.11.3 https://github.com/nlohmann/json.git

cd json || exit
cmake . -G "Unix Makefiles" -B ./build -DJSON_BuildTests=OFF -DBUILD_TESTING=OFF


TARGET_LIST+=("json")
cd ../

# YAML
# git clone -b 0.8.0 https://github.com/jbeder/yaml-cpp.git

cd yaml-cpp || exit
cmake . -G "Unix Makefiles" -B ./build -DYAML_BUILD_SHARED_LIBS=ON -DYAML_CPP_BUILD_TESTS=OFF -DBUILD_TESTING=OFF

TARGET_LIST+=("yaml-cpp")
cd ../

# lua
#if [ -d "$LUA_DIR" ]; then
#     cd "$LUA_DIR" || exit
#     cmake . -G "Unix Makefiles" -B ./build -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"/lua
#
#     TARGET_LIST+=("$LUA_DIR")
#     cd ../
#fi

# zlib
# git clone -b v1.3.1 https://github.com/madler/zlib.git

cd zlib || exit
cmake . -G "Unix Makefiles" -B ./build

TARGET_LIST+=("zlib")
cd ../

for elem in "${TARGET_LIST[@]}"; do
    cd "$elem"/build || exit
    pwd
    cmake --build . --target install --config=Release
    cd ../../
done

# protobuf
# git clone -b v29.1 https://github.com/protocolbuffers/protobuf.git

cd protobuf || exit
# git submodule update --init --recursive

cmake . -G "Unix Makefiles" -B ./build -DBUILD_TESTING=OFF -DABSL_PROPAGATE_CXX_STD=ON -DCMAKE_CXX_STANDARD=20 -Dprotobuf_BUILD_LIBPROTOC=ON -Dprotobuf_BUILD_SHARED_LIBS=ON -Dprotobuf_BUILD_TESTS=OFF

cd build || exit
cmake --build . --target install --config=Release
cd ../../

cd ../
ldconfig
echo "Third Library Build Finished."