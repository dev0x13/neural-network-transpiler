include(ExternalProject)

set(flatbuffers_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/flatbuffers)

ExternalProject_Add(
    flatbuffers
    PREFIX ${flatbuffers_PREFIX}
    URL "https://github.com/google/flatbuffers/archive/v23.5.26.tar.gz"
    URL_HASH "SHA256=1cce06b17cddd896b6d73cc047e36a254fb8df4d7ea18a46acf16c4c0cd3f3f3"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${flatbuffers_PREFIX} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DFLATBUFFERS_BUILD_TESTS=false
    LOG_UPDATE ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
)

set(FLATBUFFERS_INCLUDE_DIRS "${flatbuffers_PREFIX}/include")
set(FLATBUFFERS_FLATC_EXECUTABLE "${flatbuffers_PREFIX}/bin/flatc")
set(FLATBUFFERS_LIBRARIES "${flatbuffers_PREFIX}/lib/libflatbuffers.a")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildFlatBuffers.cmake)
