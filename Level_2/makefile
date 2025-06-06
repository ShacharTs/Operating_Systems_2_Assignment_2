CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -fprofile-arcs -ftest-coverage
LDFLAGS  = -fprofile-arcs -ftest-coverage

TARGETS = atom_warehouse atom_supplier molecule_requester molecule_supplier

# Source files
WAREHOUSE_SRC = atom_warehouse.cpp warehouse.cpp
ATOM_SUPPLIER_SRC = atom_supplier.cpp
MOLECULE_REQUESTER_SRC = molecule_requester.cpp
MOLECULE_SUPPLIER_SRC = molecule_supplier.cpp

all: $(TARGETS)

atom_warehouse: $(WAREHOUSE_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

atom_supplier: $(ATOM_SUPPLIER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

molecule_requester: $(MOLECULE_REQUESTER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

molecule_supplier: $(MOLECULE_SUPPLIER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Run gcov after manually executing the programs
coverage:
	gcov *.cpp > coverage.txt
	@echo "[Coverage written to coverage.txt]"

clean:
	rm -f $(TARGETS) *.gcno *.gcda *.gcov coverage.txt

.PHONY: all clean coverage
