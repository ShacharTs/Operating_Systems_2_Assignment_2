CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread -fprofile-arcs -ftest-coverage
LDFLAGS = -fprofile-arcs -ftest-coverage

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
	$(CXX) $(CXXFLAGS) -o $@ $(DRINK_BARS_SRC) $(LDFLAGS)

atom_supplier: $(ATOM_SUPPLIER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(ATOM_SUPPLIER_SRC) $(LDFLAGS)

molecule_supplier: $(MOLECULE_SUPPLIER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(MOLECULE_SUPPLIER_SRC) $(LDFLAGS)

molecule_requester: $(MOLECULE_REQUESTER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(MOLECULE_REQUESTER_SRC) $(LDFLAGS)

# Unified coverage
coverage: all
	./drink_bars -T 5000 -U 5001 -o 5 -c 5 -h 5 & echo $$! > server.pid
	sleep 1
	./atom_supplier -h localhost -p 5000 HYDROGEN 1 || true
	./molecule_supplier -h localhost -p 5001 WATER 1 || true
	./molecule_requester -h localhost -p 5001 WATER 1 || true
	sleep 1
	kill -INT `cat server.pid` 2>/dev/null || true
	rm -f server.pid
	gcov *.cpp > coverage.txt
	@echo "Coverage report written to coverage.txt"

clean:
	rm -f $(TARGETS) *.gcno *.gcda *.gcov coverage.txt server.pid

.PHONY: all clean coverage
