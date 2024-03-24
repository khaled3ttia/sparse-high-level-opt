CXX = g++

TEST_SRC = test.cpp
TEST_EXE = test.x
TEST_CORRECT_SRC = test_correct_ref.cpp
TEST_CORRECT_EXE = test_correct_ref.x
INCLUDE_DIR = common/include
MAIN_SRC = blocked_csr.cpp
MAIN_EXE = main.x
all: test main test_correct_ref

test:
	$(CXX) $(TEST_SRC) -I $(INCLUDE_DIR) -o $(TEST_EXE)
main:
	$(CXX) $(MAIN_SRC) -I $(INCLUDE_DIR) -o $(MAIN_EXE)
test_correct_ref:
	$(CXX) $(TEST_CORRECT_SRC) -I $(INCLUDE_DIR) -o $(TEST_CORRECT_EXE)
clean:
	rm -f *.x 
