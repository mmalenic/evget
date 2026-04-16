#ifndef EVGET_TEST_COMMON_STORE_H
#define EVGET_TEST_COMMON_STORE_H

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <vector>

#include "evget/error.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/storage/store.h"

namespace test {

/**
 * \brief A mock store that records test events and waits on notification.
 */
class StoreMock : public evget::Store {
public:
    evget::Result<void> StoreEvent(evget::Data event) override;
    void WaitForEvents(std::size_t count);
    std::vector<evget::Data> Events();

    static evget::Data MakeData();
    static evget::Data MakeKeyData(evget::DeviceType device);
    static evget::Data MakeMouseClickData(evget::DeviceType device);

private:
    std::vector<evget::Data> events_;
    std::mutex lock_;
    std::condition_variable condition_;
};

/**
 * \brief Mock store that always returns an error.
 */
class StoreErrorMock : public evget::Store {
public:
    evget::Result<void> StoreEvent(evget::Data /*event*/) override;
};

} // namespace test

#endif
