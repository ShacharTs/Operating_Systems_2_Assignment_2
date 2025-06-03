#pragma once
#include <limits>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <fstream>
#include <nlohmann/json.hpp>

enum class AtomElement { CARBON, OXYGEN, HYDROGEN };
enum class MoleculeElement { WATER, CARBON_DIOXIDE, ALCOHOL, GLUCOSE };
enum class DrinkType { SOFT_DRINK, VODKA, CHAMPAGNE };

class Warehouse {
public:
    Warehouse();
    ~Warehouse();

    void setStoragePath(const std::string& path);
    bool loadFromFile();
    bool saveToFile() const;

    bool addAtom(const std::string &atomName, uint64_t amount);
    bool makeMolecule(const std::string &moleculeName, uint64_t amount);
    uint64_t maxDrinksPossible(const std::string &drinkName);
    void printInventory() const;
    bool processCommand(const std::string &command);

private:
    AtomElement elementToEnum(const std::string &name) const;

    std::unordered_map<AtomElement, uint64_t> atomCounts;
    std::unordered_map<MoleculeElement, uint64_t> moleculeCounts;
    std::unordered_map<MoleculeElement, std::unordered_map<AtomElement, uint64_t>> moleculeRecipes;
    std::unordered_map<DrinkType, std::unordered_map<MoleculeElement, uint64_t>> drinkRecipes;

    std::string storagePath;
};
