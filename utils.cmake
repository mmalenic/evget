include(CheckIncludeFiles)
include(CheckCXXSymbolExists)

#[==========================================================================[
find_symbol
----------------

A wrapper function around ``check_cxx_symbol_exists``.

.. code:: cmake

   find_symbol(
       SYMBOL [symbol]
       FILES [files...]
       INCLUDE_DIRS [directories...]
       OUT [out_variable]
   )

Check if the given ``SYMBOL`` can be found after constructing a ``CXX`` file and
including ``FILES``. Optionally add header includes by setting the ``INCLUDE_DIRS``
argument.

Writes the cached result to ``OUT`` and defines a compilation definition macro
with the name contained in the ``OUT`` variable.
#]==========================================================================]
function(find_symbol)
    set(one_value_args OUT SYMBOL)
    set(multi_value_args FILES INCLUDE_DIRS)
    cmake_parse_arguments(FIND_SYMBOL "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    check_required_arg(FIND_SYMBOL_SYMBOL SYMBOL)
    check_required_arg(FIND_SYMBOL_OUT OUT)

    if(DEFINED ${FIND_SYMBOL_OUT})
        add_compile_definitions("${FIND_SYMBOL_OUT}=1")

        message(
            STATUS
            "utils: find_symbol result for \"${FIND_SYMBOL_SYMBOL}\" cached with value: ${${FIND_SYMBOL_OUT}}"
        )
        return()
    endif()

    if(DEFINED FIND_SYMBOL_INCLUDE_DIRS)
        set(CMAKE_REQUIRED_INCLUDES "${FIND_SYMBOL_INCLUDE_DIRS}")
    endif()

    check_cxx_symbol_exists(${FIND_SYMBOL_SYMBOL} ${FIND_SYMBOL_FILES} ${FIND_SYMBOL_OUT})

    if (${FIND_SYMBOL_OUT})
        add_compile_definitions("${FIND_SYMBOL_OUT}=1")
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
       COMPONENTS [components...]
   )

Finds a program dependency using ``find_package`` and then links it to an
existing target using ``target_link_libraries``. Treats all dependencies
and components as ``REQUIRED``.
#]==========================================================================]
function(program_dependencies TARGET DEPENDENCY_NAME)
    set(one_value_args VERSION VISIBILITY)
    set(multi_value_args COMPONENTS)
    cmake_parse_arguments(PROGRAM_DEPENDENCIES "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT ${DEPENDENCY_NAME}_FOUND)
        find_package(
            ${DEPENDENCY_NAME} ${PROGRAM_DEPENDENCIES_VERSION} REQUIRED COMPONENTS ${PROGRAM_DEPENDENCIES_COMPONENTS}
        )
    endif()

    target_link_libraries(${TARGET} ${PROGRAM_DEPENDENCIES_VISIBILITY} ${DEPENDENCY_NAME})
    message(STATUS "linked ${DEPENDENCY_NAME} to ${TARGET}")
endfunction()

#[==========================================================================[
feature_check
----------------

A wrapper function around ``check_include_files`` for ``CXX`` files.

.. code:: cmake

   feature_check(
       REQUIRES [requires...]
       INCLUDE_DIRS [directories...]
       OUT [out_variable]
   )

Check if the given ``REQUIRES`` may be included in a ``CXX`` source file.
Optionally search through additional header includes by setting the
``INCLUDE_DIRS`` argument.

Writes the cached result to ``OUT`` and defines a compilation definition macro
with the name contained in the ``OUT`` variable.
#]==========================================================================]
function(feature_check)
    set(one_value_args OUT)
    set(multi_value_args REQUIRES INCLUDE_DIRS)
    cmake_parse_arguments(FEATURE_CHECK "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    check_required_arg(FEATURE_CHECK_REQUIRES REQUIRES)
    check_required_arg(FEATURE_CHECK_OUT OUT)

    if(DEFINED ${FEATURE_CHECK_OUT})
        add_compile_definitions("${FEATURE_CHECK_OUT}=1")

        message(
            STATUS
                "utils: feature_check result for \"${FEATURE_CHECK_REQUIRES}\" cached with value: ${${FEATURE_CHECK_OUT}}"
        )
        return()
    endif()

    if(DEFINED FEATURE_CHECK_INCLUDE_DIRS)
        set(CMAKE_REQUIRED_INCLUDES "${FEATURE_CHECK_INCLUDE_DIRS}")
    endif()

    check_include_files("${FEATURE_CHECK_REQUIRES}" "${FEATURE_CHECK_OUT}" LANGUAGE CXX)

    if (${FEATURE_CHECK_OUT})
        add_compile_definitions("${FEATURE_CHECK_OUT}=1")
    endif()
endfunction()

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
