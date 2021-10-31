find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

option(USE_FMT "Use fmt library for formatting msgs." ON)

if(USE_FMT)
    find_package(fmt REQUIRED)
endif()

add_library(gui STATIC
    window.cpp
    # graphics.cpp
    ${imgui_RES_DIRS}/bindings/imgui_impl_glfw.cpp
    ${imgui_RES_DIRS}/bindings/imgui_impl_opengl3.cpp
    )

target_include_directories(gui PUBLIC ${imgui_RES_DIRS}/bindings)
target_link_libraries(gui PUBLIC imgui::imgui glfw::glfw GLEW::GLEW fmt::fmt)

if(USE_FMT)
    target_link_libraries(gui PRIVATE fmt::fmt)
    target_compile_options(gui PRIVATE -DUSE_FMT)
endif()
