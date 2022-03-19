add_program(
    TARGET ${LISTINPUTDEVICES_LIBRARY_NAME}
    FEATURE_REQUIRES fcntl.h linux/input.h
    FEATURE_CHECK LISTINPUTDEVICES_HAS_DEPENDENCIES
    SRCS
    InputDevice.cpp
    InputDeviceLister.cpp
)