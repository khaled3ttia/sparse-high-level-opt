CXX = g++
INCLUDE_DIR = ../common/include/
INCLUDE_DIR2 = ../include/
MKL_FLAGS = -lmkl_rt

all: default

default:
	$(CXX) -std=c++17 $(MKL_FLAGS) test_oo.cpp -I $(INCLUDE_DIR) -I $(INCLUDE_DIR2) -o test_oo.x
