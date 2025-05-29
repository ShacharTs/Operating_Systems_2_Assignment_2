#include <iostream>
#include <limits>
#include <sstream>
#include "wareHouse.hpp"


const std::unordered_map<std::string, AtomElement> StrToAtomElement = {
    { "CARBON", AtomElement::CARBON },
    { "OXYGEN", AtomElement::OXYGEN },
    { "HYDROGEN", AtomElement::HYDROGEN }
};

const std::unordered_map<AtomElement, std::string> AtomElementToStr = {
    { AtomElement::CARBON, "CARBON" },
    { AtomElement::OXYGEN, "OXYGEN" },
    { AtomElement::HYDROGEN, "HYDROGEN" }
};

const std::unordered_map<std::string, MoleculeElement> StrToMoleculeElement = {
    { "WATER", MoleculeElement::WATER },
    { "CARBON DIOXIDE", MoleculeElement::CARBON_DIOXIDE },
    { "ALCOHOL", MoleculeElement::ALCOHOL },
    { "GLUCOSE", MoleculeElement::GLUCOSE }
};

const std::unordered_map<MoleculeElement, std::string> MoleculeElementToStr = {
    { MoleculeElement::WATER, "WATER" },
    { MoleculeElement::CARBON_DIOXIDE, "CARBON DIOXIDE" },
    { MoleculeElement::ALCOHOL, "ALCOHOL" },
    { MoleculeElement::GLUCOSE, "GLUCOSE" }
};

Warehouse::Warehouse() {
    atomCounts[AtomElement::CARBON] = 0;
    atomCounts[AtomElement::OXYGEN] = 0;
    atomCounts[AtomElement::HYDROGEN] = 0;

    moleculeCounts[MoleculeElement::WATER] = 0;
    moleculeCounts[MoleculeElement::CARBON_DIOXIDE] = 0;
    moleculeCounts[MoleculeElement::GLUCOSE] = 0;
    moleculeCounts[MoleculeElement::ALCOHOL] = 0;



    moleculeRecipes = {
        { MoleculeElement::WATER,           {{AtomElement::HYDROGEN, 2}, {AtomElement::OXYGEN, 1}} },
        { MoleculeElement::CARBON_DIOXIDE,  {{AtomElement::CARBON, 1},   {AtomElement::OXYGEN, 2}} },
        { MoleculeElement::ALCOHOL,         {{AtomElement::CARBON, 2},   {AtomElement::HYDROGEN, 6}, {AtomElement::OXYGEN, 1}} },
        { MoleculeElement::GLUCOSE,         {{AtomElement::CARBON, 6},   {AtomElement::HYDROGEN, 12}, {AtomElement::OXYGEN, 6}} }
    };
}

Warehouse::~Warehouse() = default;

AtomElement Warehouse::elementToEnum(const std::string &name) const {
    auto it = StrToAtomElement.find(name);
    if (it == StrToAtomElement.end())
        throw std::invalid_argument("Unknown atom type: " + name);
    return it->second;
}

bool Warehouse::addAtom(const std::string &atomName, uint64_t amount) {
    try {
        AtomElement atom = elementToEnum(atomName);
        if (atomCounts[atom] > std::numeric_limits<uint64_t>::max() - amount)
            return false;
        atomCounts[atom] += amount;
        printInventory();
        return true;
    } catch (const std::invalid_argument &e) {
        std::cerr << "Add failed: " << e.what() << std::endl;
        printInventory();
        return false;
    }
}

bool Warehouse::makeMolecule(const std::string &moleculeName, uint64_t amount) {
    auto it = StrToMoleculeElement.find(moleculeName); // find molecule type
    if (it == StrToMoleculeElement.end()) {
        std::cerr << "Unknown molecule: " << moleculeName << std::endl;
        return false;
    }

    MoleculeElement molecule = it->second;
    auto recipeIt = moleculeRecipes.find(molecule); // find recipe
    if (recipeIt == moleculeRecipes.end()) {
        std::cerr << "No recipe found for molecule: " << moleculeName << std::endl;
        printInventory();
        return false;
    }

    const auto& recipe = recipeIt->second; // get atom requirements

    for (const auto& [atom, qty] : recipe) { // check available atoms
        if (atomCounts[atom] < qty * amount) {
            std::cerr << "Not enough " << qty << " x " << AtomElementToStr.at(atom)
                      << " for " << moleculeName << std::endl;
            printInventory();
            return false;
        }
    }

    for (const auto& [atom, qty] : recipe) { // deduct used atoms
        atomCounts[atom] -= qty * amount;
    }
    moleculeCounts[molecule] += amount;

    std::cout << "Created " << amount << " of " << moleculeName << std::endl;
    printInventory();
    return true;
}

void Warehouse::printInventory() const {
    std::cout << "Inventory: ";

    // Print atoms
    for (const auto& [atom, count] : atomCounts) {
        std::string name = AtomElementToStr.at(atom);
        std::cout << name << "=" << count << " ";
    }

    // Print molecules
    for (const auto& [molecule, count] : moleculeCounts) {
        std::string name = MoleculeElementToStr.at(molecule);
        std::cout << name << "=" << count << " ";
    }

    std::cout << std::endl;
}


bool Warehouse::processCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string actionStr, nameStr;
    int64_t amount;

    if (!(iss >> actionStr >> nameStr)) return false;
    if (!(iss >> amount)) return false;

    if (actionStr == "ADD") {
        return addAtom(nameStr, static_cast<uint64_t>(amount));
    }

    if (actionStr == "DELIVER") {
        return makeMolecule(nameStr, static_cast<uint64_t>(amount));
    }

    return false;
}
