find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

if (APPLE)
else (APPLE)
    set(GLFW_DEPS X11 Xrandr Xi Xinerama Xxf86vm Xcursor GL dl pthread)
endif (APPLE)

# Static Library: ImGui
add_library(${CG_IMGUI_LIB}
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/examples/opengl2_example/imgui_impl_glfw.cpp
    )
target_link_libraries(${CG_IMGUI_LIB}
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLFW_DEPS}
    )
target_include_directories(${CG_IMGUI_LIB} PRIVATE imgui)
