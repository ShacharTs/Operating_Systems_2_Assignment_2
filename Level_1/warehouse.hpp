#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>
#include "AtomElement.hpp"

class Warehouse {
public:
    Warehouse();
    ~Warehouse();

    bool addAtom(const std::string& atomName, uint64_t amount);
    void printInventory() const;
    bool processCommand(const std::string& command);

private:
    std::unordered_map<AtomElement, uint64_t> atomCounts;

    AtomElement atomStringToEnum(const std::string& name) const;
};
