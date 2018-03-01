cmake_minimum_required(VERSION 3.7)
project (neutrino)

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR} CACHE PATH "Neutrino install prefix" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

include_directories(include)

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

if(WIN32)
	include(Windows.cmake)
endif()

if(LINUX)
	include(Linux.cmake)
endif()

include(Neutrino.cmake)
include(Quark.cmake)

include(Plugin.Headers.cmake)
include(File.Input.Plugin.cmake)
include(File.Output.Plugin.cmake)
include(Trace.Evaluator.Plugin.cmake)
include(Mutator.Plugin.cmake)
include(Console.Plugin.cmake)



