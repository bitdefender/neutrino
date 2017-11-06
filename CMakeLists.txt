cmake_minimum_required(VERSION 3.7)
project (neutrino)

include_directories(include)

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

add_executable(neutrino 
	${sources}
	${platform_sources}
	${payload_sources}
	${headers}
)

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