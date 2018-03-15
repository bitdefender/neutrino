set(neutrino_headers
	src/Neutrino.Types.h
	src/Neutrino.Util.h
	src/Neutrino.Enum.Set.h
	src/Neutrino.Abstract.Translator.h

	src/Neutrino.Translator.X86.Base.h
	src/Neutrino.Translator.X86.Base.hpp
	src/Neutrino.Translator.X86.32.h
	src/Neutrino.Translator.X86.32.hpp
	src/Neutrino.Translator.X86.64.h
	src/Neutrino.Translator.X86.64.hpp

	src/Neutrino.Trampoline.X86.32.h
	src/Neutrino.Trampoline.X86.64.h

	# src/Neutrino.Translator.X86.h
	# src/Neutrino.Translator.X86.hpp
	# src/Neutrino.Translator.X64.h
	# src/Neutrino.Translator.X64.hpp
	src/Neutrino.Environment.h
	src/Neutrino.Environment.hpp
	src/Neutrino.Plugin.h
	src/Neutrino.Plugin.Manager.h
	src/Neutrino.Module.h
	src/Neutrino.Memory.h
	src/Neutrino.Heap.h
	src/Neutrino.Result.h

	src/Neutrino.Strategy.Trace.h
	src/Neutrino.Strategy.Trace.hpp

	src/Neutrino.Strategy.Trace.X86.32.h
	src/Neutrino.Strategy.Trace.X86.64.h
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

set(neutrino_sources
	src/Neutrino.Main.cpp 
	src/Neutrino.Abstract.Translator.cpp
	# src/Neutrino.Translator.cpp
	src/Neutrino.Translator.X86.Base.cpp

	src/Neutrino.Trampoline.X86.32.cpp
	src/Neutrino.Trampoline.X86.64.cpp

	# src/Neutrino.Translator.X86.cpp
	src/Neutrino.Environment.cpp
	src/Neutrino.Plugin.Manager.cpp

	src/Neutrino.Definitions.cpp

	src/Neutrino.Strategy.Trace.X86.32.cpp
	src/Neutrino.Strategy.Trace.X86.64.cpp
	src/Neutrino.Strategy.Tuple.cpp

	src/Neutrino.Simulation.Trace.cpp

	src/Neutrino.Heap.cpp

	src/Neutrino.Corpus.cpp
	src/MurmurHash3.cpp
)

set(platform_sources
    src/Neutrino.Module.${platform_src_suffix}.cpp
    src/Neutrino.Memory.${platform_src_suffix}.cpp
)

set(payload_sources
	src/http_parser.c
	src/http_parser.h
	src/Payload.cpp
	src/Buffers.cpp
	src/Payload.h
	src/Buffers.h
)

add_executable(neutrino 
	${neutrino_sources}
	${platform_sources}
#	${payload_sources}
	${neutrino_headers}
)

target_link_libraries(neutrino ${neutrino_libs})

install(TARGETS neutrino
    RUNTIME DESTINATION deploy
)

install(FILES neutrino.json
    DESTINATION deploy
)