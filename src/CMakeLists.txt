add_library(${CG_CORE_LIB}
    cg_config.cpp
    cg_logger.cpp
    cg_glfw3.cpp
    cg_keyboard.cpp
    cg_mouse.cpp
    cg_timer.cpp
    cg_window.cpp
    cg_window2d.cpp
    )

target_link_libraries(${CG_CORE_LIB}
    ${GLFW_STATIC_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )
