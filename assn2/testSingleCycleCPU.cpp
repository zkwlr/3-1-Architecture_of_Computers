#include "SingleCycleCPU.hpp"

#include <cstdio>
#include <cstdlib>

int main(int argc, char **argv)
{
  if (argc != 6)
  {
    fprintf(stderr, "Usage: %s initialPC regFileName instMemFileName dataMemFileName numCycles\n", argv[0]);
    fflush(stdout);
    exit(-1);
  }

  const std::uint32_t initialPC = (std::uint32_t)atoll(argv[1]);
  const char *regFileName = argv[2];
  const char *instMemFileName = argv[3];
  const char *dataMemFileName = argv[4];
  const std::uint64_t numCycles = (std::uint64_t)atoll(argv[5]);

  SingleCycleCPU *cpu = new SingleCycleCPU(initialPC, regFileName, instMemFileName, dataMemFileName);

  // test SingleCycleCPU::Mux
  std::bitset<32> input0(0x11111111), input1(0x10101010), output0, output1;
  std::bitset<1> selector0(0), selector1(1);
  cpu->Mux<32>(&input0, &input1, &selector0, &output0);
  cpu->Mux<32>(&input0, &input1, &selector1, &output1);
  assert(output0.to_ulong() == 0x11111111 && output1.to_ulong() == 0x10101010);

  // test SingleCycleCPU::ALU::NOR
  input0 = std::bitset<32>(0b10101010101010101010101010101010);
  input1 = std::bitset<32>(0b11110000111100001111000011110000);
  std::bitset<32> output;
  std::bitset<1> zero;
  std::bitset<4> control = std::bitset<4>(0b1100);
  cpu->ALU(&input0, &input1, &control, &output, &zero);
  std::string outputStr = output.to_string();
  assert(outputStr == "00000101000001010000010100000101");

  cpu->printPVS();
  for (size_t i = 0; i < numCycles; i++)
  {
    cpu->advanceCycle();
    cpu->printPVS();
  }

  delete cpu;

  return 0;
}
