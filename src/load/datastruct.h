#ifndef EGALITO_LOAD_DATA_STRUCT_H
#define EGALITO_LOAD_DATA_STRUCT_H

#include <vector>
#include <utility>
#include "conductor/setup.h"
#include "types.h"

class DataStructMigrator {
private:
    std::vector<std::pair<address_t, uint64_t>> fixupList;
public:
    void migrate(ConductorSetup &setup);
private:
    void commit();
};

#endif
