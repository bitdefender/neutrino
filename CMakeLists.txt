cmake_minimum_required(VERSION 3.5)
project (neutrino)

include_directories(include)

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

set(headers
	src/Neutrino.Types.h
	src/Neutrino.Translator.h
	src/Neutrino.Translator.hpp
	src/Neutrino.Environment.h
	src/Neutrino.Environment.hpp
	src/Neutrino.Plugin.h
	src/Neutrino.Plugin.Manager.h
	src/Neutrino.Module.h
	src/Neutrino.Memory.h
)

set(sources
	src/Neutrino.Main.cpp 
	src/Neutrino.Translator.cpp
	src/Neutrino.Environment.cpp
	src/Neutrino.Plugin.Manager.cpp
)

set(payload_sources
	src/http_parser.c
	src/http_parser.h
	src/Payload.cpp
	src/Buffers.cpp
	src/Payload.h
	src/Buffers.h
)

if(WIN32)
    set(platform_sources
		src/Neutrino.Module.Windows.cpp
		src/Neutrino.Memory.Windows.cpp
	)
endif()

if(LINUX)
    set(CMAKE_VERBOSE_MAKEFILE on)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -ggdb -m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ggdb -m32 -std=c++11")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

    set(platform_sources
		src/Neutrino.Module.Linux.cpp
		src/Neutrino.Memory.Linux.cpp
	)
endif()

add_executable(neutrino 
	${sources}
	${platform_sources}
	${payload_sources}
	${headers}
)

if (LINUX)
    target_link_libraries(neutrino dl stdc++fs)
endif()

set(plugin_headers
	src/Neutrino.Plugin.h
)

set(plugin_sources
	src/plugin/Neutrino.Plugin.Example.cpp
)


add_library(neutrino.plugin.example SHARED
	${plugin_sources}
	${plugin_headers}
)