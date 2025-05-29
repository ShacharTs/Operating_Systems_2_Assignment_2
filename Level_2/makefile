CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGETS = atom_warehouse atom_supplier molecule_requester molecule_supplier

# Source files
WAREHOUSE_SRC = atom_warehouse.cpp warehouse.cpp
ATOM_SUPPLIER_SRC = atom_supplier.cpp
MOLECULE_REQUESTER_SRC = molecule_requester.cpp
MOLECULE_SUPPLIER_SRC = molecule_supplier.cpp

all: $(TARGETS)

atom_warehouse: $(WAREHOUSE_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

atom_supplier: $(ATOM_SUPPLIER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

molecule_requester: $(MOLECULE_REQUESTER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

molecule_supplier: $(MOLECULE_SUPPLIER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)

.PHONY: all clean
