find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)

add_library(gui STATIC imgui.cpp)
target_link_libraries(gui PUBLIC imgui::imgui glfw::glfw)
