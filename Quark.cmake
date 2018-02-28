set(quark_sources
    src/Quark.Debugger.cpp
)

add_executable(quark
    ${quark_sources}
)

target_link_libraries(quark ${quark_libs})
