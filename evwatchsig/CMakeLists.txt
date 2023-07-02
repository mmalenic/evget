include("${CMAKE_SOURCE_DIR}/utils.cmake")

check_includes(RETURN_VAR EVWATCHSIG_HAS_FEATURE
    REQUIRES signal.h
    )

if (EVWATCHSIG_HAS_FEATURE)
    project(evwatchsig)

    add_library(
        ${PROJECT_NAME}
        include/evwatchsig/EventWatcherSignal.h
    )
    target_include_directories(${PROJECT_NAME} PUBLIC include)

    add_subdirectory(src)

    target_link_libraries(${PROJECT_NAME} PUBLIC evwatch)

    if (BUILD_TESTING)
        add_subdirectory(test)
    endif ()
endif ()