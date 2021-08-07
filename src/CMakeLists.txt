cmake_minimum_required(VERSION 3.13)

project(morfuse VERSION 1.0)

file(GLOB_RECURSE SRC "*.cpp")

find_package(BISON)
find_package(FLEX)

flex_target(mfuse-lexer ${CMAKE_CURRENT_SOURCE_DIR}/Parser/yyLexer.l ${CMAKE_CURRENT_BINARY_DIR}/generated/Parser/yyLexer.cpp DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/generated/Parser/yyLexer.hpp COMPILE_FLAGS -Cem)
bison_target(mfuse-parser ${CMAKE_CURRENT_SOURCE_DIR}/Parser/yyParser.yy ${CMAKE_CURRENT_BINARY_DIR}/generated/Parser/yyParser.cpp)
add_flex_bison_dependency(mfuse-lexer mfuse-parser)

add_library(morfuse SHARED ${SRC} ${BISON_mfuse-parser_OUTPUTS} ${FLEX_mfuse-lexer_OUTPUTS})
add_library(morfuse::morfuse ALIAS morfuse)

if(WIN32)
	target_compile_options(morfuse PRIVATE /W3)
endif()

#set_property(TARGET morfuse PROPERTY INTERPROCEDURAL_OPTIMIZATION true)

set_target_properties(morfuse PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_compile_features(morfuse PUBLIC cxx_std_17)

target_compile_definitions(morfuse PRIVATE mfuse_DLL=1)
target_include_directories(morfuse PRIVATE ${FLEX_INCLUDE_DIRS})
target_include_directories(morfuse PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(morfuse PRIVATE ${PROJECT_SOURCE_DIR}/../thirdparty/utfcpp)
target_include_directories(morfuse PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated/)

#target_include_directories(morfuse PUBLIC ${FMT_INCS})
target_link_libraries(morfuse PRIVATE 3pt)

if(WIN32)
	target_compile_definitions(morfuse PRIVATE _CRT_SECURE_NO_WARNINGS=1)
endif()

install(
	TARGETS morfuse
	DESTINATION ${CMAKE_INSTALL_PREFIX}
	EXPORT morfuse-Targets
)

install(
	DIRECTORY "${CMAKE_SOURCE_DIR}/include/morfuse"
	DESTINATION include
)

install(
	EXPORT morfuse-Targets
	DESTINATION .
	NAMESPACE mfuse::
)

set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include)
set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})

include(CMakePackageConfigHelpers)
configure_package_config_file( 
  "Config.cmake.in" 
  "morfuse-config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}
  PATH_VARS
    INCLUDE_INSTALL_DIR
	LIB_INSTALL_DIR
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/morfuse-config.cmake DESTINATION ${CMAKE_INSTALL_PREFIX})
