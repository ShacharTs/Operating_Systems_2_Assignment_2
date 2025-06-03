CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread

# Executable targets
TARGETS = drink_bars atom_supplier molecule_supplier molecule_requester

# Header and source dependencies
WAREHOUSE_SRC = warehouse.cpp
WAREHOUSE_HDR = warehouse.hpp

# Individual source files
DRINK_BARS_SRC = drink_bars.cpp $(WAREHOUSE_SRC)
ATOM_SUPPLIER_SRC = atom_supplier.cpp
MOLECULE_SUPPLIER_SRC = molecule_supplier.cpp
MOLECULE_REQUESTER_SRC = molecule_requester.cpp

# Build all
all: $(TARGETS)

drink_bars: $(DRINK_BARS_SRC) $(WAREHOUSE_HDR)
	$(CXX) $(CXXFLAGS) -o $@ $(DRINK_BARS_SRC)

atom_supplier: $(ATOM_SUPPLIER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(ATOM_SUPPLIER_SRC)

molecule_supplier: $(MOLECULE_SUPPLIER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(MOLECULE_SUPPLIER_SRC)

molecule_requester: $(MOLECULE_REQUESTER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(MOLECULE_REQUESTER_SRC)

# Clean build artifacts
clean:
	rm -f $(TARGETS)
