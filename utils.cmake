include(CheckIncludeFiles)

#[=======================================================================[
feature_check
----------------

A wrapper function around ``check_include_files`` for ``CXX`` files.

.. code:: cmake

   feature_check(
       REQUIRES [requires...]
       INCLUDE_DIRS [directories...]
       OUT
   )

Check if the given ``REQUIRES`` may be included in a ``CXX`` source file.
Optionally search through additional header includes by setting the
``INCLUDE_DIRS`` argument.

Writes the cached result to ``OUT``.
#]=======================================================================]
function(feature_check)
    set(one_value_args OUT)
    set(multi_value_args REQUIRES INCLUDE_DIRS)
    cmake_parse_arguments(FEATURE_CHECK "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    check_required_arg(FEATURE_CHECK_REQUIRES REQUIRES)
    check_required_arg(FEATURE_CHECK_OUT OUT)

    if(DEFINED ${FEATURE_CHECK_OUT})
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
endfunction()

#[=======================================================================[
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
#]=======================================================================]
macro(check_required_arg ARG ARG_NAME)
    if(NOT DEFINED ${ARG})
        message(FATAL_ERROR "utils: required parameter ${ARG_NAME} not set")
        return()
    endif()
endmacro()
