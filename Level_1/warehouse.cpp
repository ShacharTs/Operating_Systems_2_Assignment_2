#include "wareHouse.hpp"
#include <iostream>
#include <limits>
#include <sstream>

Warehouse::Warehouse() {
    atomCounts[AtomElement::CARBON] = 0;
    atomCounts[AtomElement::OXYGEN] = 0;
    atomCounts[AtomElement::HYDROGEN] = 0;
}

Warehouse::~Warehouse() = default;

AtomElement Warehouse::atomStringToEnum(const std::string &name) const {
    if (name == "CARBON") return AtomElement::CARBON;
    if (name == "OXYGEN") return AtomElement::OXYGEN;
    if (name == "HYDROGEN") return AtomElement::HYDROGEN;
    throw std::invalid_argument("Unknown atom type: " + name);
}

bool Warehouse::addAtom(const std::string &atomName, uint64_t amount) {
    try {
        AtomElement atom = atomStringToEnum(atomName);
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


void Warehouse::printInventory() const {
    std::cout << "Inventory: ";
    for (const auto& [atom, count] : atomCounts) {
        std::string name;
        switch (atom) {
            case AtomElement::CARBON:   name = "CARBON"; break;
            case AtomElement::OXYGEN:   name = "OXYGEN"; break;
            case AtomElement::HYDROGEN: name = "HYDROGEN"; break;
        }
        std::cout << name << "=" << count << " ";
    }
    std::cout << std::endl;
}

bool Warehouse::processCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string actionStr, atomStr;
    int64_t amount;

    if (!(iss >> actionStr >> atomStr)) return false;
    if (!(iss >> amount)) return false;

    if (actionStr == "ADD") {
        return addAtom(atomStr, static_cast<uint64_t>(amount));
    }

    return false;
}
