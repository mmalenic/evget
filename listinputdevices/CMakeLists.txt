include("${CMAKE_SOURCE_DIR}/utils.cmake")

check_includes(RETURN_VAR LISTINPUTDEVICES_HAS_FEATURE
    REQUIRES fcntl.h linux/input.h
    )

if (LISTINPUTDEVICES_HAS_FEATURE)
    project(listinputdevices)

    add_library(
        ${PROJECT_NAME}
        include/listinputdevices/InputDevice.h
        include/listinputdevices/InputDeviceLister.h
    )
    target_include_directories(${PROJECT_NAME} PUBLIC include)

    add_subdirectory(src)

    target_link_libraries(${PROJECT_NAME} PUBLIC clioption)

    program_dependencies(${PROJECT_NAME} fmt VISIBILITY PUBLIC)
    program_dependencies(${PROJECT_NAME} spdlog VISIBILITY PUBLIC)
    program_dependencies(${PROJECT_NAME} Boost COMPONENTS headers VISIBILITY PUBLIC)

    if (BUILD_TESTING)
        add_subdirectory(test)
    endif ()
endif ()