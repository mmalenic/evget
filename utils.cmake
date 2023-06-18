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
if ``INCLUDE_DIRS`` is defined.
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
