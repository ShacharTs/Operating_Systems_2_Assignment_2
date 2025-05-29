cmake_minimum_required(VERSION 3.10)
project(Level_1)

set(CMAKE_CXX_STANDARD 17)

# Atom warehouse
add_executable(atom_warehouse
        atom_warehouse.cpp
)

# Atom supplier
add_executable(atom_supplier
        atom_supplier.cpp
        wareHouse.hpp
        warehouse.cpp
)