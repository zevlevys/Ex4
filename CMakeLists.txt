cmake_minimum_required(VERSION 3.14)
project(Ex4)

set(CMAKE_CXX_STANDARD 14)

add_executable(Ex4 FlowExampleSimulation.cpp PhysicalMemory.cpp PhysicalMemory.h VirtualMemory.cpp VirtualMemory.h MemoryConstants.h)