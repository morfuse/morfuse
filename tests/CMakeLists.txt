cmake_minimum_required(VERSION 3.13)
project(tests)

include(CTest)

enable_testing()

file(GLOB SRCS "*.cpp")

foreach(source ${SRCS})
	get_filename_component(fileName ${source} NAME_WE)

	# Add executable with only the source file
	add_executable(${fileName} ${source})

	target_link_libraries(${fileName} morfuse)

	add_test(NAME test_${fileName} COMMAND ${fileName})

	install(
		TARGETS ${fileName}
		DESTINATION ${CMAKE_INSTALL_PREFIX}
		EXPORT test-Targets
	)
endforeach(source)

install(
	EXPORT morfuse-Targets
	DESTINATION .
	NAMESPACE mfuse::
)
