//
// Created by marki on 20/11/23.
//

#ifndef SQLITE_H
#define SQLITE_H

#include "Storage.h"
#include "../Event/Schema.h"

namespace EvgetCore {
/**
 * A storage class which stores events in an SQLite database.
 */
class SQLite : public Storage {
public:
    explicit SQLite(std::string database = "evget.db3");

    void store(Event::Data event) override;

private:
    std::reference_wrapper<std::ostream> ostream;
};
}

#endif //SQLITE_H
