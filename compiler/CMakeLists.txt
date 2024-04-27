
add_executable(cepheid)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(cepheid PRIVATE nlohmann_json::nlohmann_json)

file(GLOB_RECURSE SRC_FILES
  CONFIGURE_DEPENDS
  "src/*.cpp"
  "src/*.h"
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${SRC_FILES})

target_sources(cepheid PRIVATE ${SRC_FILES})
target_include_directories(cepheid PRIVATE src)