CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGETS = atom_warehouse atom_supplier

# Source files
WAREHOUSE_SRC = atom_warehouse.cpp warehouse.cpp
SUPPLIER_SRC = atom_supplier.cpp

all: $(TARGETS)

atom_warehouse: $(WAREHOUSE_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

atom_supplier: $(SUPPLIER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)

.PHONY: all clean
