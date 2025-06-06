CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread
COVFLAGS = -fprofile-arcs -ftest-coverage
LDFLAGS = -pthread
COVLDFLAGS = -fprofile-arcs -ftest-coverage

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
	$(CXX) $(CXXFLAGS) $(COVFLAGS) -o $@ $(DRINK_BARS_SRC) $(LDFLAGS) $(COVLDFLAGS)

atom_supplier: $(ATOM_SUPPLIER_SRC)
	$(CXX) $(CXXFLAGS) $(COVFLAGS) -o $@ $(ATOM_SUPPLIER_SRC) $(LDFLAGS) $(COVLDFLAGS)

molecule_supplier: $(MOLECULE_SUPPLIER_SRC)
	$(CXX) $(CXXFLAGS) $(COVFLAGS) -o $@ $(MOLECULE_SUPPLIER_SRC) $(LDFLAGS) $(COVLDFLAGS)

molecule_requester: $(MOLECULE_REQUESTER_SRC)
	$(CXX) $(CXXFLAGS) $(COVFLAGS) -o $@ $(MOLECULE_REQUESTER_SRC) $(LDFLAGS) $(COVLDFLAGS)

# Generate gcov reports (after running the binaries)
coverage:
	gcov *.cpp > coverage.txt
	@echo "Coverage written to coverage.txt"

# Clean build artifacts
clean:
	rm -f $(TARGETS) *.gcno *.gcda *.gcov coverage.txt

.PHONY: all clean coverage
