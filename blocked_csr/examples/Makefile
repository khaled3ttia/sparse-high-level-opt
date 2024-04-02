CXX = g++
INCLUDE_DIR = ../common/include/
INCLUDE_DIR2 = ../include/
MKL_FLAGS = -m64 -L${MKLROOT}/lib/intel64 -lmkl_intel_lp64 -lmkl_sequential -lmkl_core -lpthread -lm -ldl
#MKL_FLAGS = -lmkl_rt
all: default

default:
	$(CXX) -std=c++17 -g  test_oo.cpp -I $(INCLUDE_DIR) -I $(INCLUDE_DIR2) $(MKL_FLAGS) -o test_oo.x
