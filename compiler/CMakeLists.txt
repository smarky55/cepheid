
add_executable(cepheid)

file(GLOB_RECURSE SRC_FILES
  CONFIGURE_DEPENDS
  "src/*.cpp"
  "src/*.h"
)

target_sources(cepheid PRIVATE ${SRC_FILES})
target_include_directories(cepheid PRIVATE src)