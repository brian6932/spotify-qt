cmake_minimum_required(VERSION 3.5)

project(spotify-qt LANGUAGES CXX VERSION 3.8)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional D-Bus support
option(USE_DBUS "Use D-Bus integration" ON)

# Optional KCrash support
option(USE_KCRASH "Use KCrash integration" ON)

# Find requested Qt version
option(USE_QT6 "Force Qt 6" OFF)
option(USE_QT5 "Force Qt 5" OFF)

if (USE_QT5)
	# Forcing Qt 5
	set(QT_VERSION_MAJOR 5)
elseif (USE_QT6)
	# Forcing Qt 6
	set(QT_VERSION_MAJOR 6)
else ()
	# Auto detect, but prefer Qt 5
	find_package(Qt5 COMPONENTS Core QUIET)
	if (Qt5Core_FOUND)
		set(QT_VERSION_MAJOR 5)
	else ()
		find_package(Qt6 COMPONENTS Core QUIET)
		if (Qt6Core_FOUND)
			set(QT_VERSION_MAJOR 6)
		endif ()
	endif ()
endif ()

# Check if Qt was found
if (NOT QT_VERSION_MAJOR)
	message(FATAL_ERROR "Qt not found, please install Qt 5/6 first")
endif ()

# Prefer new OpenGL
if (POLICY CMP0072)
	cmake_policy(SET CMP0072 NEW)
endif ()

# Qt 6 requires C++17 under MSVC
if (QT_VERSION_MAJOR EQUAL 6 AND MSVC)
	set(CMAKE_CXX_STANDARD 17)
endif ()

# Set target macOS version
if (APPLE)
	# Qt 6.2 supports 10.14 and Qt 5.15 supports 10.13
	# As automated builds are only Qt 6 for now, target that version
	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")
endif ()

# Find Qt
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets Network Gui Svg REQUIRED)
if (USE_DBUS)
	find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS DBus QUIET)
endif ()
message(STATUS "Using Qt ${QT_VERSION_MAJOR}")

# Find KCRash
if (USE_KCRASH)
	find_package(KF5Crash QUIET)
endif ()

# Create main executable
if (ANDROID)
	add_library(spotify-qt SHARED ${SRC_FILES} res.qrc)
elseif (MSVC)
	add_executable(spotify-qt WIN32 ${SRC_FILES} ${LIB_SRC} res.qrc
		${CMAKE_CURRENT_SOURCE_DIR}/res/app/app.rc)
elseif (APPLE)
	set(APP_ICNS ${CMAKE_CURRENT_SOURCE_DIR}/res/app/spotify-qt.icns)
	add_executable(spotify-qt MACOSX_BUNDLE ${SRC_FILES} ${LIB_SRC} res.qrc ${APP_ICNS})
	# Load properties from Info.plist file
	set_target_properties(spotify-qt PROPERTIES
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/res/app/Info.plist)
	# Copy icon file to Resources folder
	set_source_files_properties(${APP_ICNS} PROPERTIES
		MACOSX_PACKAGE_LOCATION "Resources")
else ()
	add_executable(spotify-qt ${SRC_FILES} ${LIB_SRC} res.qrc)
endif ()

# Collect source files
add_subdirectory(src)

# Headers are in src/
target_include_directories(spotify-qt PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Get version from Git
find_package(Git QUIET)
if (GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
	execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		OUTPUT_VARIABLE GIT_COMMIT
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	target_compile_definitions(spotify-qt PRIVATE GIT_COMMIT="${GIT_COMMIT}")
endif ()

# App related definitions
target_compile_definitions(spotify-qt PRIVATE APP_VERSION="v${PROJECT_VERSION}")
target_compile_definitions(spotify-qt PRIVATE APP_NAME="${PROJECT_NAME}")
target_compile_definitions(spotify-qt PRIVATE APP_ICON="${PROJECT_NAME}")
target_compile_definitions(spotify-qt PRIVATE ORG_NAME="kraxarn")
target_compile_definitions(spotify-qt PRIVATE BUILD_TYPE="${CMAKE_BUILD_TYPE}")

# Install icons and desktop shortcut on unix-like
if (UNIX)
	install(FILES res/logo/spotify-qt.svg DESTINATION share/icons/hicolor/scalable/apps)
	install(FILES res/app/spotify-qt.desktop DESTINATION share/applications)
endif ()

if (APPLE)
	# macOS bundling
	install(TARGETS spotify-qt BUNDLE DESTINATION bin)
else ()
	# Binary for other targets
	install(TARGETS spotify-qt RUNTIME DESTINATION bin)
endif ()

# Don't show console window under windows
if (WIN32 AND NOT MSVC)
	target_link_options(spotify-qt PRIVATE -mwindows)
endif ()

# Link Qt
target_link_libraries(spotify-qt PRIVATE
	Qt${QT_VERSION_MAJOR}::Core
	Qt${QT_VERSION_MAJOR}::Widgets
	Qt${QT_VERSION_MAJOR}::Network
	Qt${QT_VERSION_MAJOR}::Gui
	Qt${QT_VERSION_MAJOR}::Svg)

# spotify-qt-lib
set(LIB_QT_IMPL ON)
add_subdirectory(lib)
target_link_libraries(spotify-qt PRIVATE spotify-qt-lib)

# Enable all compiler warnings under GCC
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(spotify-qt PRIVATE -Wall -Wextra)
endif ()

# D-Bus support is optional
if (Qt5DBus_FOUND)
	target_compile_definitions(spotify-qt PRIVATE USE_DBUS)
	target_link_libraries(spotify-qt PRIVATE Qt5::DBus)
endif ()

# KCrash support
if (KF5Crash_FOUND AND NOT USE_QT6)
	target_compile_definitions(spotify-qt PRIVATE USE_KCRASH)
	target_link_libraries(spotify-qt PRIVATE KF5::Crash)
endif ()
