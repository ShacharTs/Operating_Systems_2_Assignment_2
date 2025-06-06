#include "warehouse.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// String ↔ enum mappings for atoms
static const std::unordered_map<std::string, AtomElement> StrToAtomElement = {
    { "CARBON", AtomElement::CARBON },
    { "OXYGEN", AtomElement::OXYGEN },
    { "HYDROGEN", AtomElement::HYDROGEN }
};
static const std::unordered_map<AtomElement, std::string> AtomElementToStr = {
    { AtomElement::CARBON, "CARBON" },
    { AtomElement::OXYGEN, "OXYGEN" },
    { AtomElement::HYDROGEN, "HYDROGEN" }
};

// String ↔ enum mappings for molecules
static const std::unordered_map<std::string, MoleculeElement> StrToMoleculeElement = {
    { "WATER", MoleculeElement::WATER },
    { "CARBON_DIOXIDE", MoleculeElement::CARBON_DIOXIDE },
    { "ALCOHOL", MoleculeElement::ALCOHOL },
    { "GLUCOSE", MoleculeElement::GLUCOSE }
};
static const std::unordered_map<MoleculeElement, std::string> MoleculeElementToStr = {
    { MoleculeElement::WATER, "WATER" },
    { MoleculeElement::CARBON_DIOXIDE, "CARBON_DIOXIDE" },
    { MoleculeElement::ALCOHOL, "ALCOHOL" },
    { MoleculeElement::GLUCOSE, "GLUCOSE" }
};

// String ↔ enum mappings for drinks
static const std::unordered_map<std::string, DrinkType> StrToDrinkType = {
    { "SOFT_DRINK", DrinkType::SOFT_DRINK },
    { "VODKA", DrinkType::VODKA },
    { "CHAMPAGNE", DrinkType::CHAMPAGNE }
};
static const std::unordered_map<DrinkType, std::string> DrinkTypeToStr = {
    { DrinkType::SOFT_DRINK, "SOFT_DRINK" },
    { DrinkType::VODKA, "VODKA" },
    { DrinkType::CHAMPAGNE, "CHAMPAGNE" }
};

Warehouse::Warehouse() {
    // Initialize all atom counts to 0
    atomCounts[AtomElement::CARBON] = 0;
    atomCounts[AtomElement::OXYGEN] = 0;
    atomCounts[AtomElement::HYDROGEN] = 0;

    // Initialize all molecule counts to 0
    moleculeCounts[MoleculeElement::WATER] = 0;
    moleculeCounts[MoleculeElement::CARBON_DIOXIDE] = 0;
    moleculeCounts[MoleculeElement::ALCOHOL] = 0;
    moleculeCounts[MoleculeElement::GLUCOSE] = 0;

    // Define recipes for assembling molecules from atoms
    moleculeRecipes[MoleculeElement::WATER] = {
        { AtomElement::HYDROGEN, 2 },
        { AtomElement::OXYGEN,   1 }
    };
    moleculeRecipes[MoleculeElement::CARBON_DIOXIDE] = {
        { AtomElement::CARBON,  1 },
        { AtomElement::OXYGEN,  2 }
    };
    moleculeRecipes[MoleculeElement::ALCOHOL] = {
        { AtomElement::CARBON,  2 },
        { AtomElement::HYDROGEN,6 },
        { AtomElement::OXYGEN,  1 }
    };
    moleculeRecipes[MoleculeElement::GLUCOSE] = {
        { AtomElement::CARBON,  6 },
        { AtomElement::HYDROGEN,12 },
        { AtomElement::OXYGEN,  6 }
    };

    // Define recipes for drinks (molecule → count)
    drinkRecipes[DrinkType::SOFT_DRINK] = {
        { MoleculeElement::WATER, 1 }
    };
    drinkRecipes[DrinkType::VODKA] = {
        { MoleculeElement::ALCOHOL, 1 }
    };
    drinkRecipes[DrinkType::CHAMPAGNE] = {
        { MoleculeElement::GLUCOSE,       1 },
        { MoleculeElement::CARBON_DIOXIDE, 2 }
    };
}

Warehouse::~Warehouse() {
    // Nothing to do here; user must call saveToFile() explicitly if needed
}

void Warehouse::setStoragePath(const std::string& path) {
    storagePath = path;
}

bool Warehouse::loadFromFile() {
    if (storagePath.empty()) {
        return false;
    }

    std::ifstream infile(storagePath);
    if (!infile.good()) {
        // File does not exist or cannot be opened; leave counts at zero
        return false;
    }

    json j;
    try {
        infile >> j;
    } catch (...) {
        return false;
    }

    // Reset counts before loading
    atomCounts[AtomElement::CARBON] = 0;
    atomCounts[AtomElement::OXYGEN] = 0;
    atomCounts[AtomElement::HYDROGEN] = 0;
    moleculeCounts[MoleculeElement::WATER] = 0;
    moleculeCounts[MoleculeElement::CARBON_DIOXIDE] = 0;
    moleculeCounts[MoleculeElement::ALCOHOL] = 0;
    moleculeCounts[MoleculeElement::GLUCOSE] = 0;

    if (j.contains("atoms") && j["atoms"].is_array()) {
        for (auto& element : j["atoms"]) {
            std::string name = element.value("name", "");
            uint64_t qty = element.value("quantity", 0ULL);
            auto it = StrToAtomElement.find(name);
            if (it != StrToAtomElement.end()) {
                atomCounts[it->second] = qty;
            }
        }
    }

    if (j.contains("molecules") && j["molecules"].is_array()) {
        for (auto& element : j["molecules"]) {
            std::string name = element.value("name", "");
            uint64_t qty = element.value("quantity", 0ULL);
            auto it = StrToMoleculeElement.find(name);
            if (it != StrToMoleculeElement.end()) {
                moleculeCounts[it->second] = qty;
            }
        }
    }

    return true;
}

bool Warehouse::saveToFile() const {
    if (storagePath.empty()) {
        return false;
    }

    json j;
    j["atoms"] = json::array();
    for (auto const& pair : atomCounts) {
        json elem;
        elem["name"] = AtomElementToStr.at(pair.first);
        elem["quantity"] = pair.second;
        j["atoms"].push_back(elem);
    }

    j["molecules"] = json::array();
    for (auto const& pair : moleculeCounts) {
        json elem;
        elem["name"] = MoleculeElementToStr.at(pair.first);
        elem["quantity"] = pair.second;
        j["molecules"].push_back(elem);
    }

    std::ofstream outfile(storagePath);
    if (!outfile.good()) {
        return false;
    }

    outfile << j.dump(4);
    return true;
}

bool Warehouse::addAtom(const std::string &atomName, uint64_t amount) {
    auto it = StrToAtomElement.find(atomName);
    if (it == StrToAtomElement.end()) {
        return false;
    }
    atomCounts[it->second] += amount;
    return true;
}

bool Warehouse::makeMolecule(const std::string &moleculeName, uint64_t amount) {
    auto it = StrToMoleculeElement.find(moleculeName);
    if (it == StrToMoleculeElement.end()) {
        return false;
    }
    MoleculeElement me = it->second;

    // Ensure recipe exists
    auto recipeIt = moleculeRecipes.find(me);
    if (recipeIt == moleculeRecipes.end()) {
        return false;
    }
    const auto& recipe = recipeIt->second;

    // Check if enough atoms
    for (auto const& pair : recipe) {
        AtomElement atom = pair.first;
        uint64_t needed = pair.second * amount;
        if (atomCounts[atom] < needed) {
            std::cerr << "Not enough " << AtomElementToStr.at(atom)
                      << " to make " << amount << " of " << moleculeName << "\n";
            printInventory();
            return false;
        }
    }
    // Deduct atoms
    for (auto const& pair : recipe) {
        AtomElement atom = pair.first;
        uint64_t needed = pair.second * amount;
        atomCounts[atom] -= needed;
    }
    // Increment molecule count
    moleculeCounts[me] += amount;
    return true;
}

uint64_t Warehouse::maxDrinksPossible(const std::string &drinkName) {
    auto it = StrToDrinkType.find(drinkName);
    if (it == StrToDrinkType.end()) {
        return 0;
    }
    DrinkType dt = it->second;

    auto recipeIt = drinkRecipes.find(dt);
    if (recipeIt == drinkRecipes.end()) {
        return 0;
    }
    const auto& recipe = recipeIt->second;

    uint64_t maxCount = std::numeric_limits<uint64_t>::max();
    for (auto const& pair : recipe) {
        MoleculeElement me = pair.first;
        uint64_t needed = pair.second;
        uint64_t available = moleculeCounts.at(me);
        maxCount = std::min(maxCount, available / needed);
    }
    if (maxCount == std::numeric_limits<uint64_t>::max()) {
        return 0;
    }
    return maxCount;
}

void Warehouse::printInventory() const {
    std::cout << "Atoms:\n";
    for (auto const& pair : atomCounts) {
        std::cout << "  " << AtomElementToStr.at(pair.first)
                  << " = " << pair.second << "\n";
    }
    std::cout << "Molecules:\n";
    for (auto const& pair : moleculeCounts) {
        std::cout << "  " << MoleculeElementToStr.at(pair.first)
                  << " = " << pair.second << "\n";
    }
}

bool Warehouse::processCommand(const std::string &command) {
    std::istringstream iss(command);
    std::string action;
    iss >> action;

    if (action == "ADD_ATOM") {
        std::string atomName;
        uint64_t amount;
        iss >> atomName >> amount;
        if (addAtom(atomName, amount)) {
            saveToFile();  // <--- ADD THIS
            return true;
        }
        return false;
    }

    if (action == "DELIVER") {
        std::string moleculeName;
        uint64_t amount;
        iss >> moleculeName >> amount;
        if (makeMolecule(moleculeName, amount)) {
            saveToFile();  // <--- ADD THIS
            return true;
        }
        return false;
    }

    if (action == "GEN") {
        std::string drinkName;
        iss >> drinkName;
        uint64_t count = maxDrinksPossible(drinkName);
        std::cout << "Can generate " << count << " of " << drinkName << "\n";
        return true;
    }

    // Unrecognized command
    return false;
}


AtomElement Warehouse::elementToEnum(const std::string &name) const {
    auto it = StrToAtomElement.find(name);
    if (it == StrToAtomElement.end()) {
        return AtomElement::CARBON; // default fallback
    }
    return it->second;
}
