/**
 * \file hid_usages.h
 * \brief The HID usage names the Windows SDK exports which are required for certain compilers like MinGW.
 */

#ifndef EVGETWINDOWS_HID_USAGES_H
#define EVGETWINDOWS_HID_USAGES_H

#include <windows.h>

// This depends on windows.h, so it must stay below.
#include <hidusage.h> // IWYU pragma: export

// See
// https://learn.microsoft.com/en-us/windows-hardware/design/component-guidelines/touchscreen-required-hid-top-level-collections
// and https://learn.microsoft.com/en-us/windows-hardware/drivers/hid/hid-usages
// and https://github.com/microsoft/win32metadata/blob/main/generation/WinSDK/RecompiledIdlHeaders/shared/hidusage.h
#ifndef HID_USAGE_DIGITIZER_TOUCH_SCREEN
#define HID_USAGE_DIGITIZER_TOUCH_SCREEN ((USAGE)0x0004)
#endif

#ifndef HID_USAGE_DIGITIZER_TOUCH_PAD
#define HID_USAGE_DIGITIZER_TOUCH_PAD ((USAGE)0x0005)
#endif

#ifndef HID_USAGE_DIGITIZER_FINGER
#define HID_USAGE_DIGITIZER_FINGER ((USAGE)0x0022)
#endif

#ifndef HID_USAGE_DIGITIZER_TOUCH_VALID
#define HID_USAGE_DIGITIZER_TOUCH_VALID ((USAGE)0x0047)
#endif

#ifndef HID_USAGE_DIGITIZER_CONTACT_IDENTIFIER
#define HID_USAGE_DIGITIZER_CONTACT_IDENTIFIER ((USAGE)0x0051)
#endif

#ifndef HID_USAGE_DIGITIZER_CONTACT_COUNT
#define HID_USAGE_DIGITIZER_CONTACT_COUNT ((USAGE)0x0054)
#endif

#ifndef HID_USAGE_DIGITIZER_CONTACT_COUNT_MAXIMUM
#define HID_USAGE_DIGITIZER_CONTACT_COUNT_MAXIMUM ((USAGE)0x0055)
#endif

#ifndef HID_USAGE_DIGITIZER_SCAN_TIME
#define HID_USAGE_DIGITIZER_SCAN_TIME ((USAGE)0x0056)
#endif

#endif // EVGETWINDOWS_HID_USAGES_H
