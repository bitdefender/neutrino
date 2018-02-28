set(file_output_plugin_sources
    src/plugin/File.Output.Plugin.cpp
)

add_library(file.output.plugin SHARED
    ${file_output_plugin_sources}
    ${plugin_headers}
)
