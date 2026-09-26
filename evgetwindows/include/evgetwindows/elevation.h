/**
 * \file elevation.h
 * \brief Process elevation state.
 */

#ifndef EVGETWINDOWS_ELEVATION_H
#define EVGETWINDOWS_ELEVATION_H

#include <string>

#include "evget/error.h"

namespace evgetwindows {

/**
 * \brief Whether the token is elevated.
 * \return true if elevated
 */
[[nodiscard]] evget::Result<bool> IsElevated();

/**
 * \brief The log message describing the elevation state.
 * \param elevated whether the process is elevated
 * \return the message
 */
[[nodiscard]] std::string ElevationMessage(bool elevated);

/**
 * \brief Log the elevation state at info level.
 */
void LogElevation();

} // namespace evgetwindows

#endif // EVGETWINDOWS_ELEVATION_H
