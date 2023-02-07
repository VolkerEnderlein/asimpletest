cmake_minimum_required(VERSION 3.0)

project(asimpletest)

add_subdirectory(testing)

file(GLOB_RECURSE HEADERS_LIB include/*.h)
file(GLOB_RECURSE PRIVATE_HEADERS_LIB source/*.h)
file(GLOB_RECURSE SOURCES_LIB source/*.cpp)

add_library(asimpletestlibrary STATIC ${HEADERS_LIB} ${PRIVATE_HEADERS_LIB} ${SOURCES_LIB})
target_include_directories(asimpletestlibrary PUBLIC include)

file(GLOB_RECURSE HEADERS_TEST tests/*.h)
file(GLOB_RECURSE PRIVATE_HEADERS_TEST tests/*.h)
file(GLOB_RECURSE SOURCES_TEST tests/*.cpp)

add_executable(asimpletest_Tests ${HEADERS_TEST} ${PRIVATE_HEADERS_TEST} ${SOURCES_TEST})
target_link_libraries(asimpletest_Tests
	PUBLIC
		asimpletestlibrary
		testing
)

add_test(NAME asimpletest_Tests COMMAND asimpletest_Tests)
