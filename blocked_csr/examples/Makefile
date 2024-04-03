CXX = g++
INCLUDE_DIR = ../common/include/
INCLUDE_DIR2 = ../include/
MKL_FLAGS = -m64 -L${MKLROOT}/lib/intel64 -lmkl_intel_lp64 -lmkl_sequential -lmkl_core -lpthread -lm -ldl
#MKL_FLAGS = -lmkl_rt
all: default

default: test_correct test_timing benchmark

test_correct:
	$(CXX) -std=c++17 -g  test_correctness.cpp -I $(INCLUDE_DIR) -I $(INCLUDE_DIR2) $(MKL_FLAGS) -o test_oo.x

test_timing:
	$(CXX) -std=c++17 -g test_timing.cpp -I $(INCLUDE_DIR) -I $(INCLUDE_DIR2) $(MKL_FLAGS) -o test_time.x


benchmark:
	$(CXX) -O3 -std=c++17 -g benchmark.cpp -I $(INCLUDE_DIR) -I $(INCLUDE_DIR2) $(MKL_FLAGS) -o benchmark.x
