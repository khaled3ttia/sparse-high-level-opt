CXX = g++

TEST_SRC = test.cpp
TEST_EXE = test.x
INCLUDE_DIR = common/include
MAIN_SRC = blocked_csr.cpp
MAIN_EXE = main.x
all: test main

test:
	$(CXX) $(TEST_SRC) -I $(INCLUDE_DIR) -o $(TEST_EXE)
main:
	$(CXX) $(MAIN_SRC) -I $(INCLUDE_DIR) -o $(MAIN_EXE)
clean:
	rm -f *.x 
