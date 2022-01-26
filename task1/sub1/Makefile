CPPC := g++
CPPFLAGS := -std=c++14 -Wall -O3

SRCDIR := src
BINDIR := bin
TARGET := bin/yourcode.out 
TESTER := bin/matcomp

SRCEXT := cpp
HDREXT := hpp
SOURCES := src/matio.cpp src/nnmat.cpp src/test.cpp
HEADERS := src/nnmat.hpp src/matio.hpp
OBJECTS := bin/matio.o bin/nnmat.o bin/test.o
LIB := # nothing for now
INC := # nothing for now

TEST1ARGS := fullyconnected tests/a1a.inputmatrix.txt tests/a1a.weightmatrix.txt tests/a1a.biasmatrix.txt tests/a1a.output.txt
TEST2ARGS := activation relu tests/a2a.inputmatrix.txt tests/a2a.output.txt
TEST3ARGS := activation tanh tests/a2b.inputmatrix.txt tests/a2b.output.txt
TEST4ARGS := pooling max tests/a3a.inputmatrix.txt 2 tests/a3a.output.txt
TEST5ARGS := pooling average tests/a3b.inputmatrix.txt 2 tests/a3b.output.txt
TEST6ARGS := probability softmax tests/a4a.inputvector.txt tests/a4a.output.txt
TEST7ARGS := probability sigmoid tests/a4b.inputvector.txt tests/a4b.output.txt

$(TARGET): $(OBJECTS)
	@echo "Linking..."
	$(CPPC) $^ -o $(TARGET) $(LIB)
	@cp $(TARGET) ./yourcode.out

test: $(TARGET) $(TESTER)
	@echo "Running Test 1"
	$(TARGET) $(TEST1ARGS)
	$(TESTER) tests/a1a.outputmatrix.txt tests/a1a.output.txt

	@echo "Running Test 2"
	$(TARGET) $(TEST2ARGS)
	$(TESTER) tests/a2a.outputmatrix.txt tests/a2a.output.txt

	@echo "Running Test 3"
	$(TARGET) $(TEST3ARGS)
	$(TESTER) tests/a2b.outputmatrix.txt tests/a2b.output.txt

	@echo "Running Test 4"
	$(TARGET) $(TEST4ARGS)
	$(TESTER) tests/a3a.outputmatrix.txt tests/a3a.output.txt

	@echo "Running Test 5"
	$(TARGET) $(TEST5ARGS)
	$(TESTER) tests/a3b.outputmatrix.txt tests/a3b.output.txt

	@echo "Running Test 6"
	$(TARGET) $(TEST6ARGS)
	$(TESTER) tests/a4a.outputvector.txt tests/a4a.output.txt

	@echo "Running Test 7"
	$(TARGET) $(TEST7ARGS)
	$(TESTER) tests/a4b.outputvector.txt tests/a4b.output.txt

	@echo "All tests passed successfully"

$(TESTER): $(BINDIR)/nnmat.o $(BINDIR)/matio.o $(BINDIR)/matcomp.o
	@echo "Creating matcomp"
	$(CPPC) $^ -o $(TESTER) $(LIB)

$(BINDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BINDIR)
	$(CPPC) $(CPPFLAGS) $(INC) -c $< -o $@ 

clean:
	@echo "Cleaning..."
	rm -r $(BINDIR)

# Tests
#tester:
#  $(CC) $(CFLAGS) test/tester.cpp $(INC) $(LIB) -o bin/tester

# Spikes
#ticket:
#  $(CC) $(CFLAGS) spikes/ticket.cpp $(INC) $(LIB) -o bin/ticket

.PHONY: clean
	

