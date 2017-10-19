cmake_minimum_required(VERSION 3.7)
project (neutrino)

include_directories(include)

set(headers
	src/Neutrino.Translator.h
	src/Neutrino.Plugin.h
	src/Neutrino.Plugin.Manager.h
	src/Neutrino.Module.h
)

set(sources
	src/Neutrino.Main.cpp 
	src/Neutrino.Translator.cpp
	src/Neutrino.Plugin.Manager.cpp
)

if(WIN32)
    set(platform_sources
		src/Neutrino.Module.Windows.cpp
	)
endif()

add_executable(neutrino 
	${sources}
	${platform_sources}
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