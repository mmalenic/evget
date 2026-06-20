set(LIBRARY_NAME libevgetwindows)
set(EVGETWINDOWS_LIBRARY_NAME
    "${LIBRARY_NAME}"
    PARENT_SCOPE
)

set(INCLUDE include/evgetwindows)
set(SRC src)

add_library(${LIBRARY_NAME})
target_sources(
    ${LIBRARY_NAME}
    PRIVATE ${SRC}/backend.cpp
    PUBLIC FILE_SET
           HEADERS
           BASE_DIRS
           include
           FILES
           ${INCLUDE}/backend.h
)
set_property(TARGET ${LIBRARY_NAME} PROPERTY OUTPUT_NAME evgetwindows)

target_link_libraries(${LIBRARY_NAME} PUBLIC libevget)

toolbelt_add_dep(
    ${LIBRARY_NAME}
    spdlog
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    spdlog::spdlog
    FIND_PACKAGE_ARGS
    REQUIRED
)
toolbelt_add_dep(
    ${LIBRARY_NAME}
    Boost
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    Boost::headers
    FIND_PACKAGE_ARGS
    REQUIRED
)

if(BUILD_TESTING)
    target_sources(${TEST_EXECUTABLE_NAME} PUBLIC test/compiler_support.cpp)
    target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC test)
endif()

if(EVGET_INSTALL_LIB)
    install(TARGETS ${LIBRARY_NAME} FILE_SET headers)
endif()
