/**
 * \file data.h
 * \brief Event data container for managing collections of event entries.
 */

#ifndef EVGET_EVENT_DATA_H
#define EVGET_EVENT_DATA_H

#include <vector>

#include "evget/event/entry.h"

namespace evget {
/**
 * \brief Data represents the actual entries that are generated from device events. The order
 *        of the entries defines the final insertion order in the storage component.
 */
class Data {
public:
    /**
     * \brief Get a reference to the entries.
     * \return Entries reference
     */
    [[nodiscard]] const std::vector<Entry>& Entries() const;

    /**
     * \brief Merge with another data object by extending the entries of this object.
     * \param data the data object to merge with
     */
    void MergeWith(Data&& data);

    /**
     * \brief Take the entries out of this object
     * \return Entries moved out of an object
     */
    std::vector<Entry> IntoEntries() &&;

    /**
     * \brief Add an entry.
     * \param entry entry to insert
     */
    void AddEntry(Entry entry);

    /**
     * \brief If there are any entries in this data.
     * \return boolean indicating emptiness
     */
    [[nodiscard]] bool Empty() const;

private:
    std::vector<Entry> entries_;
};
} // namespace evget

#endif
