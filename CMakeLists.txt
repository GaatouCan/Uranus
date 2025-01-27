cmake_minimum_required(VERSION 3.29)
project(Uranus VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

if (WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00)
    option(USE_SOLUTION_FOLDERS ON)
endif ()

add_compile_definitions(ASIO_STANDALONE)
add_compile_definitions(ASIO_HAS_CO_AWAIT)

# Import Third Library
set(THIRD_LIBRARY_DIR D:/library/install)
set(CMAKE_INSTALL_PREFIX D:/library/install/Uranus)

if (WIN32)
    list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/spdlog)
    list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/asio)
    list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/nlohmann_json)
    list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/protobuf)
    list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/zlib)
    list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/YAML_CPP)
    list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/mysql-connector-c++-9.1.0-winx64)
else ()
    list(APPEND CMAKE_PREFIX_PATH ~/cpp_server/library/install/mysql-connector-c++-9.1.0)
endif ()

find_package(spdlog CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(mysql-concpp CONFIG REQUIRED)
#find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Setting Protobuf Generated Directory
message(STATUS "Using Protobuf ${Protobuf_VERSION}")

if (CMAKE_CROSSCOMPILING)
    find_program(PROTOBUF_PROTOC protoc)
else ()
    set(PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif ()

set(PROTOBUF_DEFINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/def)
set(PROTOBUF_GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/gen)
set(PROTOBUF_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/lib)

# Add All Protobuf Files
file(GLOB_RECURSE PROTOBUF_LIST ${PROTOBUF_DEFINE_DIR}/*.proto)

# Generate *.pb.h & *.pb.cc File And Generate Binary *.proto.lib File
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROTOBUF_LIBRARY_DIR})
foreach (proto ${PROTOBUF_LIST})
    get_filename_component(proto_name ${proto} NAME_WE)
    set(proto_src ${PROTOBUF_GENERATED_DIR}/${proto_name}.pb.cc)
    set(proto_hdr ${PROTOBUF_GENERATED_DIR}/${proto_name}.pb.h)
    add_custom_command(
            OUTPUT ${proto_src} ${proto_hdr}
            COMMAND ${PROTOBUF_PROTOC}
            ARGS --cpp_out ${PROTOBUF_GENERATED_DIR} -I ${PROTOBUF_DEFINE_DIR} ${proto}
            DEPENDS ${proto})

    list(APPEND PROTOBUF_FILES ${proto_src} ${proto_hdr})
endforeach ()

add_library(proto_static STATIC ${PROTOBUF_FILES})
target_link_libraries(proto_static PRIVATE protobuf::libprotobuf)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB_RECURSE WORLD_SRC
        ${CMAKE_SOURCE_DIR}/world/include/*.h
        ${CMAKE_SOURCE_DIR}/world/source/*.cpp
)

if (MSVC)
    foreach(file ${WORLD_SRC})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

set(CMAKE_DEBUG_POSTFIX d)

add_library(Uranus SHARED ${WORLD_SRC})

target_include_directories(Uranus PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/world/include>
        $<INSTALL_INTERFACE:include>
)


target_link_libraries(Uranus PUBLIC spdlog::spdlog)
target_link_libraries(Uranus PUBLIC asio::asio)
target_link_libraries(Uranus PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(Uranus PUBLIC protobuf::libprotobuf)
target_link_libraries(Uranus PUBLIC yaml-cpp::yaml-cpp)
#target_link_libraries(base_static PUBLIC mysql::concpp)

file(GLOB_RECURSE WORLD_HEADER_FILE ${CMAKE_SOURCE_DIR}/world/include/*.h)
set_target_properties(Uranus PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        PUBLIC_HEADER ${WORLD_HEADER_FILE}
)

# 安装目标
install(TARGETS Uranus
        EXPORT UranusTargets                   # 导出目标信息，用于生成配置文件
        RUNTIME DESTINATION bin                # 动态库文件 (Windows .dll 或 Linux/macOS 可执行文件)
        LIBRARY DESTINATION lib                # 动态库文件 (Linux/macOS .so)
        ARCHIVE DESTINATION lib                # 静态库文件 (Windows .lib 或 Linux/macOS .a)
        PUBLIC_HEADER DESTINATION include      # 公共头文件
)

# 安装头文件
install(DIRECTORY world/include/ DESTINATION include FILES_MATCHING PATTERN "*.h")
#install(DIRECTORY world/include/ TYPE INCLUDE)
# 安装 CMake 配置文件
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/UranusConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

# 配置生成 AConfig.cmake 文件
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/UranusConfig.cmake.in"   # 输入模板
        "${CMAKE_CURRENT_BINARY_DIR}/UranusConfig.cmake"            # 输出配置文件
        INSTALL_DESTINATION lib/cmake/Uranus                        # 安装路径
)

# 安装配置文件
install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/UranusConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/UranusConfigVersion.cmake"
        DESTINATION lib/cmake/Uranus
)

# 导出安装目标
install(EXPORT UranusTargets
        FILE UranusTargets.cmake
        NAMESPACE Uranus::
        DESTINATION lib/cmake/Uranus
)

unset(CMAKE_DEBUG_POSTFIX )

add_executable(world world/world.cpp)
target_include_directories(world PRIVATE ${CMAKE_SOURCE_DIR}/world/include)
target_link_libraries(world PUBLIC Uranus)

file(GLOB_RECURSE COMMON_SRC    ${CMAKE_SOURCE_DIR}/server/common/*.h      ${CMAKE_SOURCE_DIR}/server/common/*.cpp     ${CMAKE_SOURCE_DIR}/server/common/*.cc)
file(GLOB_RECURSE IMPL_SRC      ${CMAKE_SOURCE_DIR}/server/impl/*.h        ${CMAKE_SOURCE_DIR}/server/impl/*.cpp       ${CMAKE_SOURCE_DIR}/server/impl/*.cc)
file(GLOB_RECURSE COMMAND_SRC   ${CMAKE_SOURCE_DIR}/server/command/*.h     ${CMAKE_SOURCE_DIR}/server/command/*.cpp    ${CMAKE_SOURCE_DIR}/server/command/*.cc)
file(GLOB_RECURSE PLAYER_SRC    ${CMAKE_SOURCE_DIR}/server/player/*.h      ${CMAKE_SOURCE_DIR}/server/player/*.cpp     ${CMAKE_SOURCE_DIR}/server/player/*.cc)
file(GLOB_RECURSE CONFIG_SRC    ${CMAKE_SOURCE_DIR}/server/config/*.h      ${CMAKE_SOURCE_DIR}/server/config/*.cpp     ${CMAKE_SOURCE_DIR}/server/config/*.cc)
#file(GLOB_RECURSE WORLD_SRC     ${CMAKE_SOURCE_DIR}/server/world/*.h       ${CMAKE_SOURCE_DIR}/server/world/*.cpp      ${CMAKE_SOURCE_DIR}/server/world/*.cc)

set(SERVER_SRC server/server.h server/GameServer.h server/GameServer.cpp ${COMMON_SRC} ${IMPL_SRC} ${COMMAND_SRC} ${CONFIG_SRC} ${PLAYER_SRC})
if (MSVC)
    foreach(file ${SERVER_SRC})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

add_library(server SHARED ${SERVER_SRC})
target_include_directories(server PRIVATE ${PROTOBUF_GENERATED_DIR})
target_link_libraries(server PRIVATE proto_static)
target_link_libraries(server PUBLIC Uranus)

#add_executable(server ${SERVER_SRC})

#target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/base)
#target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/struct)
#target_link_libraries(server PRIVATE base_static)

