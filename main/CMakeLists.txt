function(cg_add_source_file TARGET_NAME SOURCE_NAME)
    set(${TARGET_NAME}_SOURCE_LIST ${${TARGET_NAME}_SOURCE_LIST}
        ${SOURCE_NAME} PARENT_SCOPE)
endfunction()

function(cg_add_program PROGRAM_NAME)
    set(PROGRAM_TARGET ${PROGRAM_NAME})
    message(STATUS "Creating program '${PROGRAM_NAME}' with sources:")
    foreach(source ${ARGN})
        message(STATUS "   ${source}")
        cg_add_source_file(${PROGRAM_NAME} ${source})
    endforeach()

    add_executable(${PROGRAM_TARGET} ${${PROGRAM_NAME}_SOURCE_LIST})
    target_link_libraries(${PROGRAM_TARGET}
        ${CG_ADDITIONAL_LIBRARIES}
        ${CG_CORE_LIB}
        ${CG_IMGUI_LIB}
        )
    install(TARGETS ${PROGRAM_TARGET} DESTINATION ${PROJECT_SOURCE_DIR}/bin)
endfunction()

include_directories(".")

set(CG_ADDITIONAL_LIBRARIES ${SDL2_LIBRARY} ${FFTWF_LIBRARIES})
cg_add_program("wave-gui"
    main.cpp
    app.cpp
    core.cpp
    ui.cpp
    data.cpp
    )
