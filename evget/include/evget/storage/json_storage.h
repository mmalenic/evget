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
 * A storage class which prints events to stdout.
 */
class JsonStorage : public Store {
public:
    explicit JsonStorage(std::unique_ptr<std::ostream> ostream);
    explicit JsonStorage(std::unique_ptr<std::ostream, std::function<void(std::ostream*)>> ostream);

    Result<void> StoreEvent(Data event) override;

private:
    std::variant<std::unique_ptr<std::ostream>, std::unique_ptr<std::ostream, std::function<void(std::ostream*)>>>
        ostream_;
};
} // namespace evget

#endif
