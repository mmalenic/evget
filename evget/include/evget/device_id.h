/**
 * \file device_id.h
 * \brief Maps device keys to UUIDs.
 */

#ifndef EVGET_DEVICE_ID_H
#define EVGET_DEVICE_ID_H

#include <boost/uuid/name_generator_sha1.hpp>
#include <boost/uuid/namespaces.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <string>
#include <unordered_map>

namespace evget {

/**
 * \brief A helper class to map device keys to generated UUIDs.
 * \tparam Key the type used to identify a device
 */
template <typename Key>
class DeviceId {
public:
    /**
     * \brief Get the UUID for a device, generating a random one on first access.
     * \param key the device identifier
     * \return reference to the device UUID string
     */
    const std::string& Uuid(Key key);

    /**
     * \brief Get a deterministic UUID for a device, derived from the key on first access.
     * \param key the device identifier hashed into the UUID
     * \return reference to the device UUID string
     */
    [[nodiscard]] const std::string& UuidDeterministic(const Key& key);

private:
    std::unordered_map<Key, std::string> device_uuids_;
};

template <typename Key>
const std::string& DeviceId<Key>::Uuid(Key key) {
    auto [iterator, inserted] = device_uuids_.try_emplace(key);
    if (inserted) {
        iterator->second = boost::uuids::to_string(boost::uuids::random_generator()());
    }
    return iterator->second;
}

template <typename Key>
const std::string& DeviceId<Key>::UuidDeterministic(const Key& key) {
    auto [iterator, inserted] = device_uuids_.try_emplace(key);
    if (inserted) {
        const boost::uuids::name_generator_sha1 generator{boost::uuids::ns::url()};
        iterator->second = boost::uuids::to_string(generator(key));
    }
    return iterator->second;
}

} // namespace evget

#endif // EVGET_DEVICE_ID_H
