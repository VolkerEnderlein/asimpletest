cmake_minimum_required(VERSION 3.0)

project(testing)

set(USE_CATCH ON)

macro (config_h PROJECT_NAME1)
	set(CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME1}/config.h")
	if(USE_CATCH)
		set(CONF_STRING "#pragma once\n\n#define USE_CATCH")
	else()
		set(CONF_STRING "#pragma once")
	endif(USE_CATCH)

	set(OVERWRITE_VERSION_H 1)
	if (EXISTS ${CONFIG_FILE})
		file(READ ${CONFIG_FILE} VERSION_H_STRING)
		string(COMPARE NOTEQUAL "${VERSION_H_STRING}" "${CONF_STRING}" OVERWRITE_VERSION_H)
	endif()
	if (${OVERWRITE_VERSION_H})
		file(WRITE ${CONFIG_FILE} "${CONF_STRING}")
		message(STATUS "Writing config.h file")
	endif()
endmacro()

config_h(testing)

# -----------------------------------------------------------------------------
# ----------------- VS-Project "testing" --------------------------------------
# -----------------------------------------------------------------------------

file(GLOB_RECURSE HEADERS_LIB include/*.h)
file(GLOB_RECURSE HPP_HEADERS_LIB include/*.hpp)
file(GLOB_RECURSE SOURCES_LIB source/*.cpp)

add_library(testing STATIC ${HEADERS_LIB} ${HPP_HEADERS_LIB} ${SOURCES_LIB})
target_include_directories(testing PUBLIC include)
