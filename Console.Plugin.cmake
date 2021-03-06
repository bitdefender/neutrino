set(console_plugin_sources
    src/plugin/Console.Logger.Plugin.cpp
)

add_library(console.logger.plugin SHARED
    ${console_plugin_sources}
    ${plugin_headers}
)


install(TARGETS console.logger.plugin
    RUNTIME DESTINATION deploy/plugins
    LIBRARY DESTINATION deploy/plugins
)
