/**
 * \file backend.h
 * \brief Windows input backend.
 */

#ifndef EVGETWINDOWS_BACKEND_H
#define EVGETWINDOWS_BACKEND_H

#include <memory>

#include "evget/error.h"
#include "evget/storage/store.h"

namespace evgetwindows {

/**
 * \brief Represents the Windows backend.
 */
class Backend {
public:
    /**
     * \brief Create the Windows backend.
     * \param storage the event storage
     * \return the backend, or an error if the Windows backend is not yet implemented.
     */
    static evget::Result<std::unique_ptr<Backend>> Create(evget::Store& storage);

    Backend(const Backend&) = delete;
    Backend(Backend&&) = delete;
    Backend& operator=(const Backend&) = delete;
    Backend& operator=(Backend&&) = delete;
    ~Backend() = default;

private:
    Backend() = default;
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_BACKEND_H
