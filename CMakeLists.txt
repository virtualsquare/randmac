cmake_minimum_required(VERSION 3.12)
project(randmac
		VERSION 0.0.1
		DESCRIPTION "generate random MAC addresses"
		HOMEPAGE_URL "https://github.com/virtualsquare/randmac"
		LANGUAGES C)

set(HEADERS_REQUIRED strcase.h)
include(CheckIncludeFile)

foreach(HEADER IN LISTS HEADERS_REQUIRED)
  check_include_file(${HEADER} ${HEADER}_OK)
  if(NOT ${HEADER}_OK)
    message(FATAL_ERROR "header file ${HEADER} not found")
  endif()
endforeach(HEADER)

include(GNUInstallDirs)
add_definitions(-D_GNU_SOURCE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FORTIFY_SOURCE=2 -O2 -pedantic -Wall -Wextra")
add_executable(randmac randmac.c)
install(TARGETS randmac
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_subdirectory(man)

add_custom_target(uninstall
  "${CMAKE_COMMAND}"
    -DCADO_SPOOL_DIR=${CADO_SPOOL_DIR}
    -P "${PROJECT_SOURCE_DIR}/Uninstall.cmake"
  )
