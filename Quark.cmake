set(quark_headers
    src/Quark.Test.Extractor.h
    src/Quark.Debugger.h
)

set(quark_sources
    src/Quark.Main.cpp
    src/Quark.Debugger.${platform_src_suffix}.cpp
    src/Quark.Test.Extractor.${platform_src_suffix}.cpp
)

add_executable(quark
    ${quark_headers}
    ${quark_sources}
)

target_link_libraries(quark ${quark_libs})

install(TARGETS quark
    RUNTIME DESTINATION deploy
)
