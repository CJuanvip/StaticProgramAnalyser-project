file(GLOB srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB headers
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")
add_executable(autotester ${srcs})
target_link_libraries(autotester spa ${AUTOTESTER_LIBRARIES})
target_link_libraries(autotester -no-pie)
if (NOT WIN32)
    target_link_libraries(autotester pthread)
endif()
