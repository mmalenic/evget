/**
 * \file json_storage.h
 * \brief JSON storage implementation for outputting events in JSON format.
 */

#ifndef EVGET_STORAGE_JSON_STORAGE_H
#define EVGET_STORAGE_JSON_STORAGE_H

#include <functional>
#include <memory>
#include <ostream>
#include <variant>

#include "evget/error.h"
#include "evget/event/data.h"
#include "evget/storage/store.h"

namespace evget {

/**
 * \brief A storage class which prints events to stdout.
 */
class JsonStorage : public Store {
public:
    /**
     * \brief Construct a `JsonStorage` with a standard output stream.
     * \param ostream unique pointer to the output stream
     */
    explicit JsonStorage(std::unique_ptr<std::ostream> ostream);
    
    /**
     * \brief Construct a `JsonStorage` with a custom deleter output stream.
     * \param ostream unique pointer to the output stream with custom deleter
     */
    explicit JsonStorage(std::unique_ptr<std::ostream, std::function<void(std::ostream*)>> ostream);

    Result<void> StoreEvent(Data event) override;

private:
    std::variant<std::unique_ptr<std::ostream>, std::unique_ptr<std::ostream, std::function<void(std::ostream*)>>>
        ostream_;
};
} // namespace evget

#endif
