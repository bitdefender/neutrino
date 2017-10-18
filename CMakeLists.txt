cmake_minimum_required(VERSION 3.7)
project (neutrino)

include_directories(include)

set(headers
	src/Neutrino.Translator.h
	src/Neutrino.Plugin.h
	src/Neutrino.Plugin.Manager.h
)

set(sources
	src/Neutrino.Main.cpp 
	src/Neutrino.Translator.cpp
	src/Neutrino.Plugin.Manager.cpp
)

add_executable(neutrino 
	${sources}
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