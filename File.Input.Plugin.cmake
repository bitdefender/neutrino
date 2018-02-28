set(file_input_plugin_sources
    src/plugin/File.Input.Plugin.cpp
)

add_library(file.input.plugin SHARED
    ${file_input_plugin_sources}
    ${plugin_headers}
)
