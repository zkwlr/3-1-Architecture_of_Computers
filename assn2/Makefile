SRCS = RegisterFile.cpp Memory.cpp SingleCycleCPU.cpp

.PHONY: all
all: testSingleCycleCPU

testSingleCycleCPU: $(SRCS) testSingleCycleCPU.cpp
	g++ -std=c++11 -o $@ $^

.PHONY: clean
clean:
	rm -f testSingleCycleCPU

