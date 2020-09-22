file(GLOB srcs
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ast/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ast/extractors/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/pkb/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/parser/*.cpp")
file(GLOB headers
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ast/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/ast/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ast/extractors/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/ast/extractors/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/pkb/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/pkb/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/parser/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/parser/*.hpp")
add_executable(unit_testing ${srcs})
target_link_libraries(unit_testing spa)
