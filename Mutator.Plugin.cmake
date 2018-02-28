set(mutator_plugin_sources
    src/plugin/Mutator.Plugin.cpp
)

add_library(mutator.plugin SHARED
    ${mutator_plugin_sources}
    ${plugin_headers}
)
