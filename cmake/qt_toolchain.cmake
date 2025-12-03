# cmake/qt_toolchain.cmake
# Toolchain for locating Qt6 on macOS (tailored to /Users/doyoung/Qt layout).
# Usage:
#  cmake -S . -B build -G "Unix Makefiles" \
#    -DCMAKE_TOOLCHAIN_FILE=$(pwd)/cmake/qt_toolchain.cmake \
#    -DQT_INSTALL_PREFIX=/Users/doyoung/Qt
#
# This file:
#  - defaults QT_INSTALL_PREFIX to /Users/doyoung/Qt (override with -DQT_INSTALL_PREFIX=...)
#  - tries several locations (including /Users/doyoung/Qt/6.5.3/macos/lib/cmake) to find Qt6
#  - sets Qt6_DIR / CMAKE_PREFIX_PATH for find_package
#  - exposes Qt bin in PATH during configuration and sets DYLD_LIBRARY_PATH on macOS

if(NOT DEFINED QT_INSTALL_PREFIX)
  if(DEFINED ENV{QT_INSTALL_PREFIX})
    set(QT_INSTALL_PREFIX $ENV{QT_INSTALL_PREFIX} CACHE PATH "Qt install prefix (from environment)")
  else()
    set(QT_INSTALL_PREFIX "/Users/doyoung/Qt" CACHE PATH "Qt install prefix")
  endif()
else()
  set(QT_INSTALL_PREFIX "${QT_INSTALL_PREFIX}" CACHE PATH "Qt install prefix" FORCE)
endif()

message(STATUS "qt_toolchain: QT_INSTALL_PREFIX = ${QT_INSTALL_PREFIX}")

# Common candidate paths we try (explicit known path first)
set(_explicit_known "${QT_INSTALL_PREFIX}/6.5.3/macos/lib/cmake/Qt6")
set(_fmt_candidates "")
if(EXISTS "${_explicit_known}")
  set(Qt6_DIR "${_explicit_known}" CACHE PATH "Qt6 CMake dir (explicit known location)" FORCE)
  message(STATUS "qt_toolchain: using explicit known Qt6_DIR = ${Qt6_DIR}")
else()
  # check common locations under prefix
  if(EXISTS "${QT_INSTALL_PREFIX}/lib/cmake/Qt6")
    set(Qt6_DIR "${QT_INSTALL_PREFIX}/lib/cmake/Qt6" CACHE PATH "Qt6 CMake dir" FORCE)
    message(STATUS "qt_toolchain: found Qt6 under ${QT_INSTALL_PREFIX}/lib/cmake/Qt6")
  else()
    # try versioned subdirs like /Users/doyoung/Qt/<version>/<arch>/lib/cmake/Qt6
    file(GLOB _found_qt6_dirs LIST_DIRECTORIES true "${QT_INSTALL_PREFIX}/*/macos/lib/cmake/Qt6" "${QT_INSTALL_PREFIX}/*/*/lib/cmake/Qt6")
    if(_found_qt6_dirs)
      list(GET _found_qt6_dirs 0 _first_qt6_dir)
      set(Qt6_DIR "${_first_qt6_dir}" CACHE PATH "Qt6 CMake dir (auto-detected)" FORCE)
      message(STATUS "qt_toolchain: auto-detected Qt6_DIR = ${Qt6_DIR}")
    endif()
  endif()
endif()

if(DEFINED Qt6_DIR AND EXISTS "${Qt6_DIR}")
  # ensure parent lib/cmake is included in CMAKE_PREFIX_PATH
  get_filename_component(_qt_cmake_parent "${Qt6_DIR}" DIRECTORY) # .../lib/cmake
  list(APPEND CMAKE_PREFIX_PATH "${_qt_cmake_parent}" "${Qt6_DIR}")
  message(STATUS "qt_toolchain: appended ${_qt_cmake_parent} and ${Qt6_DIR} to CMAKE_PREFIX_PATH")
else()
  message(WARNING "qt_toolchain: Qt6 CMake dir not found automatically. "
                  "You can pass -DQt6_DIR=/path/to/lib/cmake/Qt6 or set -DQT_INSTALL_PREFIX to the correct Qt root.")
endif()

# Make sure CMAKE_PREFIX_PATH is visible to find_package
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" CACHE STRING "Paths for find_package" FORCE)

# Find qmake / tools if present (helps some projects/tools)
find_program(QMAKE_EXECUTABLE qmake HINTS
  "${QT_INSTALL_PREFIX}/6.5.3/macos/bin"
  "${QT_INSTALL_PREFIX}/bin"
  "${QT_INSTALL_PREFIX}/*/macos/bin"
  NO_DEFAULT_PATH)
if(QMAKE_EXECUTABLE)
  set(QMAKE_EXECUTABLE "${QMAKE_EXECUTABLE}" CACHE FILEPATH "qmake executable from Qt" FORCE)
  message(STATUS "qt_toolchain: found qmake: ${QMAKE_EXECUTABLE}")
endif()

# Export Qt bin into configure-time PATH and set macOS DYLD_LIBRARY_PATH for configure-time processes
set(_qt_bin_path "${QT_INSTALL_PREFIX}/6.5.3/macos/bin")
if(NOT EXISTS "${_qt_bin_path}")
  # fallback to common bin locations
  set(_qt_bin_path "${QT_INSTALL_PREFIX}/bin")
  if(NOT EXISTS "${_qt_bin_path}")
    # try auto-detected Qt6_DIR parent/bin
    if(DEFINED _qt_cmake_parent)
      get_filename_component(_maybe_prefix "${_qt_cmake_parent}" DIRECTORY) # .../lib
      get_filename_component(_maybe_prefix "${_maybe_prefix}" DIRECTORY)      # .../<arch>
      set(_candidate_bin "${_maybe_prefix}/bin")
      if(EXISTS "${_candidate_bin}")
        set(_qt_bin_path "${_candidate_bin}")
      endif()
    endif()
  endif()
endif()

if(EXISTS "${_qt_bin_path}")
  if(WIN32)
    set(ENV{PATH} "${_qt_bin_path};$ENV{PATH}")
  else()
    set(ENV{PATH} "${_qt_bin_path}:$ENV{PATH}")
    if(APPLE)
      if(DEFINED ENV{DYLD_LIBRARY_PATH})
        set(ENV{DYLD_LIBRARY_PATH} "${QT_INSTALL_PREFIX}/6.5.3/macos/lib:$ENV{DYLD_LIBRARY_PATH}")
      else()
        set(ENV{DYLD_LIBRARY_PATH} "${QT_INSTALL_PREFIX}/6.5.3/macos/lib")
      endif()
      message(STATUS "qt_toolchain: prepended ${_qt_bin_path} to PATH and set DYLD_LIBRARY_PATH for configure-time")
    else()
      if(DEFINED ENV{LD_LIBRARY_PATH})
        set(ENV{LD_LIBRARY_PATH} "${QT_INSTALL_PREFIX}/lib:$ENV{LD_LIBRARY_PATH}")
      else()
        set(ENV{LD_LIBRARY_PATH} "${QT_INSTALL_PREFIX}/lib")
      endif()
      message(STATUS "qt_toolchain: prepended ${_qt_bin_path} to PATH and set LD_LIBRARY_PATH for configure-time")
    endif()
  endif()
else()
  message(WARNING "qt_toolchain: Qt bin directory not found under expected locations; configure-time PATH not modified.")
endif()

# End of toolchain