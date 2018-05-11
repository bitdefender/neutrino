#set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -m32 -std=c++11")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

set(platform_src_suffix Linux)
add_definitions(-D_BUILD_LINUX)

set(file_input_plugin_libs
    stdc++fs
)

set(neutrino_libs
    dl 
    stdc++fs
)

set(quark_libs
    stdc++fs
)
