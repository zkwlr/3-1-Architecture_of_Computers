SRCS = RegisterFile.cpp Memory.cpp

.PHONY: all
all: testPipelinedCPU

testPipelinedCPU: testPipelinedCPU.cpp PipelinedCPU.cpp $(SRCS)
	g++ -std=c++11 -o $@ $^
# -m32

.PHONY: clean
clean:
	rm -f testPipelinedCPU

