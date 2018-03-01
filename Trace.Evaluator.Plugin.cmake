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

install(TARGETS trace.evaluator.plugin
    RUNTIME DESTINATION deploy/plugins
)

