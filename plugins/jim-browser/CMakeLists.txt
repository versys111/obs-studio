cmake_minimum_required(VERSION 2.8.12)
project(jim-browser)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/UI/obs-frontend-api")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/deps")

find_package(CEF QUIET)

if(NOT CEF_FOUND)
	message(STATUS "CEF Not found -- jim-browser plugin disabled.")
	return()
endif()

if(WIN32)
	option(BROWSER_SHARED_TEXTURE_SUPPORT_ENABLED "Enable shared texture support for the browser plugin" OFF)

	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/browser-config.h.in"
		"${CMAKE_CURRENT_BINARY_DIR}/browser-config.h")
endif()

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# ----------------------------------------------------------------------------

set(jim-browser_LIBRARIES
	libobs
	obs-frontend-api
	)

if(NOT APPLE)
	list(APPEND jim-browser_LIBRARIES
		${CEF_LIBRARIES})
	include_directories(${CEF_ROOT_DIR})

	if(MSVC)
		string(REPLACE "/MD" "/MT"
			"CMAKE_C_FLAGS"
			"${CMAKE_C_FLAGS}")

		string(REPLACE "/MD" "/MT"
			"CMAKE_CXX_FLAGS"
			"${CMAKE_CXX_FLAGS}")

		string(TOUPPER "${CMAKE_CONFIGURATION_TYPES}" UPPER_CONFIG_TYPES)
		foreach(CONFIG_TYPE ${UPPER_CONFIG_TYPES})
			string(REPLACE "/MD" "/MT"
				"CMAKE_C_FLAGS_${CONFIG_TYPE}"
				"${CMAKE_C_FLAGS_${CONFIG_TYPE}}")

			string(REPLACE "/MD" "/MT"
				"CMAKE_CXX_FLAGS_${CONFIG_TYPE}"
				"${CMAKE_CXX_FLAGS_${CONFIG_TYPE}}")
		endforeach()
	endif()
else()
	list(APPEND jim-browser_LIBRARIES
		${COCOA_FRAMEWORK}
		${APPKIT_FRAMEWORK}
		${IOSURFACE_FRAMEWORK})
endif()

set(jim-browser_SOURCES
	jim-browser/jim-browser-plugin.cpp
	jim-browser/jim-browser-source.cpp
	jim-browser/jim-browser-source-windows.cpp
	jim-browser/browser-scheme.cpp
	jim-browser/browser-client.cpp
	jim-browser/browser-app.cpp
	deps/json11/json11.cpp
	deps/base64/base64.cpp
	wide-string.cpp
	)
set(jim-browser_HEADERS
	jim-browser/jim-browser-source.hpp
	jim-browser/browser-scheme.hpp
	jim-browser/browser-client.hpp
	jim-browser/browser-app.hpp
	browser-version.h
	deps/json11/json11.hpp
	deps/base64/base64.hpp
	cef-headers.hpp
	wide-string.hpp
	)

add_library(jim-browser MODULE
	${jim-browser_SOURCES}
	${jim-browser_HEADERS}
	)

target_link_libraries(jim-browser
	${jim-browser_LIBRARIES}
	)

# ----------------------------------------------------------------------------

set(cef-jimstrap_SOURCES
	cef-jimstrap/cef-jimstrap-main.cpp
	jim-browser/browser-app.cpp
	deps/json11/json11.cpp
	)
set(cef-jimstrap_HEADERS
	cef-jimstrap/cef-jimstrap-main.cpp
	jim-browser/browser-app.hpp
	deps/json11/json11.hpp
	cef-headers.hpp
	)

add_executable(cef-jimstrap
	${cef-jimstrap_SOURCES}
	${cef-jimstrap_HEADERS}
	)
target_link_libraries(cef-jimstrap
	${CEF_LIBRARIES}
	)

if (APPLE)
	set_target_properties(cef-jimstrap PROPERTIES
		COMPILE_FLAGS "-mmacosx-version-min=10.8")
endif(APPLE)

if (WIN32)
	set_target_properties(cef-jimstrap PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif(WIN32)

if (APPLE AND XCODE)
	set_xcode_property(cef-jimstrap CLANG_CXX_LIBRARY "libc++")
endif(APPLE AND XCODE)

# ----------------------------------------------------------------------------

if (APPLE)
	# TODO
else()
	math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")
	add_custom_command(TARGET jim-browser POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CEF_ROOT_DIR}/Resources"
		"${CMAKE_BINARY_DIR}/rundir/$<CONFIGURATION>/obs-plugins/${BITS}bit/"
	)

	add_custom_command(TARGET jim-browser POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"${CEF_ROOT_DIR}/Release/chrome_elf.dll"
		"${CMAKE_BINARY_DIR}/rundir/$<CONFIGURATION>/obs-plugins/${BITS}bit/"
		COMMAND ${CMAKE_COMMAND} -E copy
		"${CEF_ROOT_DIR}/Release/libcef.dll"
		"${CMAKE_BINARY_DIR}/rundir/$<CONFIGURATION>/obs-plugins/${BITS}bit/"
		COMMAND ${CMAKE_COMMAND} -E copy
		"${CEF_ROOT_DIR}/Release/libEGL.dll"
		"${CMAKE_BINARY_DIR}/rundir/$<CONFIGURATION>/obs-plugins/${BITS}bit/"
		COMMAND ${CMAKE_COMMAND} -E copy
		"${CEF_ROOT_DIR}/Release/libGLESv2.dll"
		"${CMAKE_BINARY_DIR}/rundir/$<CONFIGURATION>/obs-plugins/${BITS}bit/"
		COMMAND ${CMAKE_COMMAND} -E copy
		"${CEF_ROOT_DIR}/Release/natives_blob.bin"
		"${CMAKE_BINARY_DIR}/rundir/$<CONFIGURATION>/obs-plugins/${BITS}bit/"
		COMMAND ${CMAKE_COMMAND} -E copy
		"${CEF_ROOT_DIR}/Release/snapshot_blob.bin"
		"${CMAKE_BINARY_DIR}/rundir/$<CONFIGURATION>/obs-plugins/${BITS}bit/"
	)

	install_obs_plugin_with_data(jim-browser data)
	install_obs_plugin(cef-jimstrap)
endif()
