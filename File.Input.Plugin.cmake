set(file_input_plugin_sources
    src/plugin/File.Input.Plugin.cpp
)

add_library(file.input.plugin SHARED
    ${file_input_plugin_sources}
    ${plugin_headers}
)

target_link_libraries(file.input.plugin ${file_input_plugin_libs})

install(TARGETS file.input.plugin
    RUNTIME DESTINATION deploy/plugins
    LIBRARY DESTINATION deploy/plugins
)

