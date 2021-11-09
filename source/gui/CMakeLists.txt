find_package(imgui REQUIRED)
find_package(imguizmo REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

if(OPTION_USE_FMT)
    find_package(fmt REQUIRED)
endif()

add_library(gui STATIC
    window.cpp
    # graphics.cpp
    ${imgui_RES_DIRS}/bindings/imgui_impl_glfw.cpp
    ${imgui_RES_DIRS}/bindings/imgui_impl_opengl3.cpp
    )

target_include_directories(gui PUBLIC ${imgui_RES_DIRS}/bindings)
target_link_libraries(gui PUBLIC imgui::imgui glfw::glfw GLEW::GLEW)
target_link_libraries(gui PUBLIC imguizmo::imguizmo)

if(OPTION_USE_FMT)
    target_link_libraries(gui PUBLIC fmt::fmt)
    target_compile_options(gui PUBLIC -DUSE_FMT)
endif()
