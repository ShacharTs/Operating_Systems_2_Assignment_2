CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -fprofile-arcs -ftest-coverage
LDFLAGS = -fprofile-arcs -ftest-coverage

TARGETS = atom_warehouse atom_supplier
WAREHOUSE_SRC = atom_warehouse.cpp warehouse.cpp
SUPPLIER_SRC = atom_supplier.cpp

all: $(TARGETS)

atom_warehouse:
	$(CXX) $(CXXFLAGS) -o atom_warehouse $(WAREHOUSE_SRC) $(LDFLAGS)

atom_supplier:
	$(CXX) $(CXXFLAGS) -o atom_supplier $(SUPPLIER_SRC) $(LDFLAGS)

coverage: all
	./atom_warehouse -f warehouse.json -T 5000 -U 5001 & echo $$! > pid
	sleep 1
	./atom_supplier localhost HYDROGEN 3
	sleep 1
	kill -INT `cat pid` || true
	rm -f pid
	gcov *.cpp > coverage.txt
	@echo "[Coverage written to coverage.txt]"

clean:
	rm -f $(TARGETS) *.gcno *.gcda *.gcov coverage.txt pid

.PHONY: all clean coverage
