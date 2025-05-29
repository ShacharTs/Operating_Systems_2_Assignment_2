CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGETS =  molecule_supplier atom_warehouse

all: $(TARGETS)

atom_warehouse: atom_warehouse.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

molecule_supplier: molecule_supplier.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)

.PHONY: all clean
