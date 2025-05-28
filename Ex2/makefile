CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGETS =  molecule_supplier

all: $(TARGETS)

molecule_supplier: molecule_supplier.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)

.PHONY: all clean
