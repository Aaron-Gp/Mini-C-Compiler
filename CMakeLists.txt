cmake_minimum_required(VERSION 3.16)
project(lab4)
set(CMAKE_CXX_STANDARD 14)

include(FindFLEX)
include(FindBISON)
if(FLEX_FOUND)
 message("Info: flex found!")
else()
 message("Error: flex not found!")
endif()
if(BISON_FOUND)
 message("Info: bison found!")
else()
 message("Error: bison not found!")
endif()

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

BISON_TARGET(Parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
FLEX_TARGET(Scanner lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

add_compile_options(-pedantic)
# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

add_executable(Compilerlab4
 main.cpp
 ast.cpp
 common.h
 ${BISON_Parser_OUTPUTS}
 ${FLEX_Scanner_OUTPUTS}
)
target_compile_features(Compilerlab4 PRIVATE cxx_std_14)
target_link_libraries(Compilerlab4 ${FLEX_LIBRARIES})