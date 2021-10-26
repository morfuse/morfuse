cmake_minimum_required(VERSION 3.13)

project(morfuse VERSION 1.0)

file(GLOB_RECURSE SRC "*.cpp")

find_package(BISON)
find_package(FLEX)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated/Parser/)
flex_target(mfuse-lexer ${CMAKE_CURRENT_SOURCE_DIR}/Parser/yyLexer.l ${CMAKE_CURRENT_BINARY_DIR}/generated/Parser/yyLexer.cpp DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/generated/Parser/yyLexer.hpp COMPILE_FLAGS -Cem)
bison_target(mfuse-parser ${CMAKE_CURRENT_SOURCE_DIR}/Parser/yyParser.yy ${CMAKE_CURRENT_BINARY_DIR}/generated/Parser/yyParser.cpp)
add_flex_bison_dependency(mfuse-lexer mfuse-parser)

add_library(morfuse SHARED ${SRC} ${BISON_mfuse-parser_OUTPUTS} ${FLEX_mfuse-lexer_OUTPUTS})
add_library(morfuse::morfuse ALIAS morfuse)

if(DEBUG_MEM)
	target_compile_definitions(morfuse PUBLIC "_DEBUG_MEMBLOCK=1")
endif()

if(WIN32)
	target_compile_options(morfuse PRIVATE "/W3")
	target_compile_definitions(morfuse PRIVATE "_CRT_SECURE_NO_WARNINGS=1")
elseif(UNIX)
	target_compile_options(morfuse PRIVATE "-Wall" "-Wextra")
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_options(morfuse PRIVATE "-fvisibility-global-new-delete-hidden")
	endif()
	set_target_properties(morfuse PROPERTIES
		C_VISIBILITY_PRESET hidden
		CXX_VISIBILITY_PRESET hidden
	)

	set_target_properties(morfuse PROPERTIES VISIBILITY_INLINES_HIDDEN 1)
endif()

if(NOT ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug"))
	include(CheckIPOSupported)
	check_ipo_supported(RESULT ipo_supported OUTPUT error)
	if(ipo_supported)
		# currently not unix because clang doesn't like it (invalid record)
		message(STATUS "Interprocedural optimization enabled")
		set_property(TARGET morfuse PROPERTY INTERPROCEDURAL_OPTIMIZATION true)
	endif()
endif()

target_compile_features(morfuse PUBLIC cxx_std_17)

#set_target_properties(morfuse PROPERTIES
#	CXX_STANDARD 17
#	CXX_STANDARD_REQUIRED ON
#	CXX_EXTENSIONS OFF
#)

target_compile_definitions(morfuse PRIVATE mfuse_DLL=1)
target_include_directories(morfuse PRIVATE ${FLEX_INCLUDE_DIRS})
target_include_directories(morfuse PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(morfuse PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated/)

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
