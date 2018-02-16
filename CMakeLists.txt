cmake_minimum_required(VERSION 3.5)
project (neutrino)

include_directories(include)

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

set(headers
	src/Neutrino.Types.h
	src/Neutrino.Enum.Set.h
	src/Neutrino.Abstract.Translator.h
	src/Neutrino.Translator.h
	src/Neutrino.Translator.hpp
	src/Neutrino.Environment.h
	src/Neutrino.Environment.hpp
	src/Neutrino.Plugin.h
	src/Neutrino.Plugin.Manager.h
	src/Neutrino.Module.h
	src/Neutrino.Memory.h
	src/Neutrino.Heap.h
	src/Neutrino.Result.h

	src/Neutrino.Strategy.Trace.h
	src/Neutrino.Strategy.Tuple.h
	src/Neutrino.Simulation.Trace.h

	src/Neutrino.Test.h

	src/Neutrino.Queue.h
	src/Neutrino.Queue.hpp
	src/Neutrino.Priority.Queue.h
	src/Neutrino.Priority.Queue.hpp
	src/Neutrino.Fair.Queue.h
	src/Neutrino.Fair.Queue.hpp

	src/Neutrino.Plugin.h
	src/Neutrino.Input.Plugin.h
	src/Neutrino.Output.Plugin.h
	src/Neutrino.Evaluator.Plugin.h
	src/Neutrino.Mutator.Plugin.h
	src/Neutrino.Logger.Plugin.h

	src/Neutrino.Bloom.Filter.h
	src/Neutrino.Bloom.Filter.hpp
	src/Neutrino.Corpus.h

	src/MurmurHash3.h
	src/TinySHA1.h
)

set(sources
	src/Neutrino.Main.cpp 
	src/Neutrino.Translator.cpp
	src/Neutrino.Environment.cpp
	src/Neutrino.Plugin.Manager.cpp

	src/Neutrino.Strategy.Trace.cpp
	src/Neutrino.Strategy.Tuple.cpp

	src/Neutrino.Simulation.Trace.cpp

	src/Neutrino.Heap.cpp

	src/Neutrino.Corpus.cpp
	src/MurmurHash3.cpp
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

set(debugger_sources
	src/Quark.Debugger.cpp
)

add_executable(quark
	${debugger_sources}
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

set(file_input_plugin_sources
	src/plugin/File.Input.Plugin.cpp
)

add_library(file.input.plugin SHARED
	${file_input_plugin_sources}
	${plugin_headers}
)

set(file_output_plugin_sources
	src/plugin/File.Output.Plugin.cpp
)

add_library(file.output.plugin SHARED
	${file_output_plugin_sources}
	${plugin_headers}
)


set(virtual_memory
	src/modules/Module.Layout.h
	src/modules/Module.Layout.Windows.cpp
	src/modules/Module.Layout.Linux.cpp
)

set(trace_evaluator_plugin_sources
	src/plugin/Trace.Evaluator.Plugin.cpp
)

add_library(trace.evaluator.plugin SHARED
	${trace_evaluator_plugin_sources}
	${plugin_headers}
	${virtual_memory}
)

set(mutator_plugin_sources
	src/plugin/Mutator.Plugin.cpp
)

add_library(mutator.plugin SHARED
	${mutator_plugin_sources}
	${plugin_headers}
)

set(console_plugin_sources
	src/plugin/Console.Logger.Plugin.cpp
)

add_library(console.logger.plugin SHARED
	${console_plugin_sources}
	${plugin_headers}
)

