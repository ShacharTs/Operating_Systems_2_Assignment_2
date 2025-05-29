#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>
#include "AtomElement.hpp"
#include "MoleculeElement.hpp"


class Warehouse {
public:
    Warehouse();
    ~Warehouse();

    bool addAtom(const std::string& atomName, uint64_t amount);
    bool makeMolecule(const std::string& moleculeName, uint64_t amount);

    void printInventory() const;
    bool processCommand(const std::string& command);

private:
    std::unordered_map<AtomElement, uint64_t> atomCounts;
    std::unordered_map<MoleculeElement, uint64_t> moleculeCounts;
    std::unordered_map<MoleculeElement, std::unordered_map<AtomElement, uint64_t>> moleculeRecipes;

    AtomElement elementToEnum(const std::string& name) const;
};
