include(CheckIncludeFiles)
include(CheckCXXSymbolExists)

#[==========================================================================[
check_symbol
----------------

A wrapper function around ``check_cxx_symbol_exists``.

.. code:: cmake

   check_symbol(
       SYMBOL [symbol]
       FILES [files...]
       INCLUDE_DIRS [directories...]
       RETURN_VAR [return_variable]
   )

Check if the given ``SYMBOL`` can be found after constructing a ``CXX`` file and
including ``FILES``. Optionally add header includes by setting the ``INCLUDE_DIRS``
argument.

Writes the cached result to ``RETURN_VAR`` and defines a compilation definition macro
with the name contained in the ``RETURN_VAR`` variable.
#]==========================================================================]
function(check_symbol)
    set(one_value_args RETURN_VAR SYMBOL)
    set(multi_value_args FILES INCLUDE_DIRS)
    cmake_parse_arguments(CHECK_SYMBOL "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    check_required_arg(CHECK_SYMBOL_SYMBOL SYMBOL)
    check_required_arg(CHECK_SYMBOL_RETURN_VAR RETURN_VAR)
    check_required_arg(CHECK_SYMBOL_FILES FILES)

    prepare_check_function(CHECK_SYMBOL_RETURN_VAR CHECK_SYMBOL_INCLUDE_DIRS)

    check_cxx_symbol_exists(${CHECK_SYMBOL_SYMBOL} ${CHECK_SYMBOL_FILES} ${CHECK_SYMBOL_RETURN_VAR})

    if(${CHECK_SYMBOL_RETURN_VAR})
        add_compile_definitions("${CHECK_SYMBOL_RETURN_VAR}=1")
    endif()
endfunction()

#[==========================================================================[
program_dependencies
----------------

Adds program dependencies using ``find_package`` and ``target_link_libraries``.

.. code:: cmake

   program_dependencies(
       <TARGET>
       <DEPENDENCY_NAME>
       VERSION [version]
       VISIBILITY [visibility]
       COMPONENTS [components...]
       LINK_COMPONENTS [link_components...]
   )

Finds a program dependency using ``find_package`` and then links it to an
existing target using ``target_link_libraries``. Treats all dependencies
and components as ``REQUIRED``. ``LINK_COMPONENTS`` optionally specifies the
the components that should be linked to the target, and if not present defaults
to ``COMPONENTS``. ``DIRECT_LINK`` species linking a dependency as
``${DEPENDENCY_NAME}`` rather than ``${DEPENDENCY_NAME}::${DEPENDENCY_NAME}``.
#]==========================================================================]
function(program_dependencies TARGET DEPENDENCY_NAME)
    set(options DIRECT_LINK)
    set(one_value_args VERSION VISIBILITY)
    set(multi_value_args COMPONENTS LINK_COMPONENTS)
    cmake_parse_arguments(PROGRAM_DEPENDENCIES "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT ${DEPENDENCY_NAME}_FOUND)
        find_package(
            ${DEPENDENCY_NAME} ${PROGRAM_DEPENDENCIES_VERSION} REQUIRED COMPONENTS ${PROGRAM_DEPENDENCIES_COMPONENTS}
        )
    endif()

    if(NOT DEFINED PROGRAM_DEPENDENCIES_LINK_COMPONENTS)
        set(PROGRAM_DEPENDENCIES_LINK_COMPONENTS ${PROGRAM_DEPENDENCIES_COMPONENTS})
    endif()

    if(DEFINED PROGRAM_DEPENDENCIES_LINK_COMPONENTS)
        list(LENGTH PROGRAM_DEPENDENCIES_LINK_COMPONENTS COMPONENTS_LENGTH)
        math(EXPR LOOP "${COMPONENTS_LENGTH} - 1")

        foreach(INDEX RANGE 0 ${LOOP})
            list(GET PROGRAM_DEPENDENCIES_LINK_COMPONENTS ${INDEX} COMP)
            target_link_libraries(${TARGET} ${PROGRAM_DEPENDENCIES_VISIBILITY} ${DEPENDENCY_NAME}::${COMP})
        endforeach()
    else()
        if(PROGRAM_DEPENDENCIES_DIRECT_LINK)
            target_link_libraries(${TARGET} ${PROGRAM_DEPENDENCIES_VISIBILITY} ${DEPENDENCY_NAME})
        else()
            target_link_libraries(${TARGET} ${PROGRAM_DEPENDENCIES_VISIBILITY} ${DEPENDENCY_NAME}::${DEPENDENCY_NAME})
        endif()
    endif()

    message(STATUS "utils: linked ${DEPENDENCY_NAME} to ${TARGET}")
endfunction()

#[==========================================================================[
check_includes
----------------

A wrapper function around ``check_include_files`` for ``CXX`` files.

.. code:: cmake

   check_includes(
       REQUIRES [requires...]
       INCLUDE_DIRS [directories...]
       RETURN_VAR [return_variable]
   )

Check if the given ``REQUIRES`` may be included in a ``CXX`` source file.
Optionally search through additional header includes by setting the
``INCLUDE_DIRS`` argument.

Writes the cached result to ``RETURN_VAR`` and defines a compilation definition macro
with the name contained in the ``RETURN_VAR`` variable.
#]==========================================================================]
function(check_includes)
    set(one_value_args RETURN_VAR)
    set(multi_value_args REQUIRES INCLUDE_DIRS)

    cmake_parse_arguments(CHECK_INCLUDES "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    check_required_arg(CHECK_INCLUDES_REQUIRES REQUIRES)
    check_required_arg(CHECK_INCLUDES_RETURN_VAR RETURN_VAR)

    prepare_check_function(CHECK_INCLUDES_RETURN_VAR CHECK_INCLUDES_INCLUDE_DIRS)

    check_include_files("${CHECK_INCLUDES_REQUIRES}" "${CHECK_INCLUDES_RETURN_VAR}" LANGUAGE CXX)

    if(${CHECK_INCLUDES_RETURN_VAR})
        add_compile_definitions("${CHECK_INCLUDES_RETURN_VAR}=1")
    endif()
endfunction()

#[==========================================================================[
prepare_check_function
----------------

A macro which is used within ``check_includes`` and ``check_symbol`` to set up
common logic and variables.

.. code:: cmake

   prepare_check_function(
       <RETURN_VAR>
       <INCLUDE_DIRS>
   )

Returns early if ``RETURN_VAR`` is defined. Sets ``CMAKE_REQUIRED_INCLUDES``
if ``INCLUDE_DIRS`` is defined. Assumes that ``RETURN_VAR`` and ``INCLUDE_DIRS``
is passed as a variable name and not a variable value.
#]==========================================================================]
macro(prepare_check_function RETURN_VAR INCLUDE_DIRS)
    if(DEFINED ${${RETURN_VAR}})
        add_compile_definitions("${${RETURN_VAR}}=1")

        message(STATUS "utils: check result for \"${${RETURN_VAR}}\" cached with value: ${${${RETURN_VAR}}}")
        return()
    endif()

    if(DEFINED ${INCLUDE_DIRS})
        set(CMAKE_REQUIRED_INCLUDES "${${INCLUDE_DIRS}}")
    endif()
endmacro()

#[==========================================================================[
setup_testing
----------------

A macro which sets up testing for an executable.

.. code:: cmake

   setup_testing(
       <TEST_EXECUTABLE_NAME>
       <LIBRARY_NAME>
   )

Enabled testing and links ``GTest`` to ``TEST_EXECUTABLE_NAME``. Links ``LIBRARY_NAME``
to ``TEST_EXECUTABLE_NAME``.
#]==========================================================================]
macro(setup_testing TEST_EXECUTABLE_NAME LIBRARY_NAME)
    target_link_libraries(${TEST_EXECUTABLE_NAME} PUBLIC ${LIBRARY_NAME})

    include(GoogleTest)
    enable_testing()

    program_dependencies(
        ${TEST_EXECUTABLE_NAME}
        GTest
        COMPONENTS
        gtest
        gtest_main
        gmock
        VISIBILITY
        PUBLIC
    )

    set(gtest_force_shared_crt
        ON
        CACHE BOOL "" FORCE
    )

    if(TARGET ${TEST_EXECUTABLE_NAME})
        gtest_discover_tests(${TEST_EXECUTABLE_NAME})
    endif()
endmacro()

#[==========================================================================[
check_required_arg
----------------

A macro which is used to check for required ``cmake_parse_arguments``
arguments.

.. code:: cmake

   check_required_arg(
       <ARG>
       <ARG_NAME>
   )

Check if ``ARG`` is defined, printing an error message with ``ARG_NAME``
and returning early if not.
#]==========================================================================]
macro(check_required_arg ARG ARG_NAME)
    if(NOT DEFINED ${ARG})
        message(FATAL_ERROR "utils: required parameter ${ARG_NAME} not set")
        return()
    endif()
endmacro()

#[==========================================================================[
create_header_file
----------------

A function which creates a header file containing to contents of a ```file_name``.

.. code:: cmake

   create_header_file(
       <TARGET_FILE_NAME>
       <HEADER_FILE_NAME>
       <VARIABLE_NAME>
   )

Read ``TARGET_FILE_NAMES`` and create a string_view with their contents inside
``HEADER_FILE_NAME`` with the name ``VARIABLE_NAME`` and namespace ``NAMESPACE``.
#]==========================================================================]
function(create_header_file HEADER_FILE_NAME VARIABLE_NAME NAMESPACE)
    set(multi_value_args TARGET_FILE_NAMES)
    cmake_parse_arguments(CREATE_HEADER_FILE "" "" "${multi_value_args}" ${ARGN})

    check_required_arg(CREATE_HEADER_FILE_TARGET_FILE_NAMES TARGET_FILE_NAMES)

    cmake_path(GET HEADER_FILE_NAME STEM HEADER_STEM)
    string(TOUPPER "${HEADER_STEM}" HEADER_STEM)
    string(TOUPPER "${NAMESPACE}" NAMESPACE_UPPER)
    string(REPLACE "::" "_" NAMESPACE_UPPER ${NAMESPACE_UPPER})

    foreach(TARGET_FILE_NAME IN LISTS CREATE_HEADER_FILE_TARGET_FILE_NAMES)
        file(STRINGS "${TARGET_FILE_NAME}" LINES)
        foreach(LINE IN LISTS LINES)
            string(STRIP "${LINE}" LINE)
            set(OUTPUT_STRING "${OUTPUT_STRING}\"${LINE}\"\n")
        endforeach()
    endforeach()
    string(STRIP "${OUTPUT_STRING}" OUTPUT_STRING)

    string(CONCAT HEADER
            "// Auto-generated file.\n\n"
            "#ifndef ${NAMESPACE_UPPER}_${HEADER_STEM}_H\n"
            "#define ${NAMESPACE_UPPER}_${HEADER_STEM}_H\n\n"
            "#include <string_view>\n\n"
            "namespace ${NAMESPACE} {\n"
            "    using namespace std::literals;\n\n"
            "    inline constexpr auto ${VARIABLE_NAME} = ${OUTPUT_STRING}sv;\n"
            "} // ${NAMESPACE}\n\n"
            "#endif // ${NAMESPACE_UPPER}_${HEADER_STEM}_H"
    )

    file(WRITE "${HEADER_FILE_NAME}" "${HEADER}")
endfunction()