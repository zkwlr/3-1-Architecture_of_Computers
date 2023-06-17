#ifndef __SINGLE_CYCLE_CPU_HPP__
#define __SINGLE_CYCLE_CPU_HPP__

#include "Memory.hpp"
#include "RegisterFile.hpp"

class SingleCycleCPU
{
public:
  SingleCycleCPU(
      const std::uint32_t initialPC, const char *regFileName,
      const char *instMemFileName, const char *dataMemFileName) : m_PC(initialPC),
                                                                  m_registerFile(new RegisterFile(regFileName)),
                                                                  m_instMemory(new Memory(Memory::LittleEndian, instMemFileName)),
                                                                  m_dataMemory(new Memory(Memory::LittleEndian, dataMemFileName)),
                                                                  m_currCycle(0) {}
  void printPVS()
  {
    printf("==================== Cycle %llu ====================\n", m_currCycle);
    printf("PC = 0x%08lx\n", m_PC.to_ulong());
    printf("Registers:\n");
    m_registerFile->printRegisters();
    printf("Data Memory:\n");
    m_dataMemory->printMemory();
    printf("Instruction Memory:\n");
    m_instMemory->printMemory();
  }
  void advanceCycle();

private:
  std::bitset<32> m_PC; // the Program Counter (PC) register
  // sequential circuits
  RegisterFile *m_registerFile; // the Register File (Registers)
  Memory *m_instMemory;         // the Instruction Memory
  Memory *m_dataMemory;         // the Data Memory
public:
  // the AND gate
  template <size_t BitWidth>
  void AND(
      const std::bitset<BitWidth> *input0, const std::bitset<BitWidth> *input1,
      std::bitset<BitWidth> *output);
  // the Adder unit
  template <size_t BitWidth>
  void Add(
      const std::bitset<BitWidth> *input0, const std::bitset<BitWidth> *input1,
      std::bitset<BitWidth> *output);
  // the 2-to-1 Multiplexer unit
  template <size_t BitWidth>
  void Mux(
      const std::bitset<BitWidth> *input0, const std::bitset<BitWidth> *input1,
      const std::bitset<1> *select,
      std::bitset<BitWidth> *output);
  // the SignExtend unit
  template <size_t InputBitWidth, size_t OutputBitWidth>
  void SignExtend(
      const std::bitset<InputBitWidth> *input,
      std::bitset<OutputBitWidth> *output);
  // the Shift-Left-2 unit
  template <size_t BitWidth>
  void ShiftLeft2(
      const std::bitset<BitWidth> *input,
      std::bitset<BitWidth> *output);
  // the 32-bit Arithmetic-Logic Unit (ALU)
  void ALU(
      const std::bitset<32> *input0, const std::bitset<32> *input1,
      const std::bitset<4> *control,
      std::bitset<32> *output, std::bitset<1> *zero);
  // the Control unit
  void Control(
      const std::bitset<6> *opcode,
      std::bitset<1> *regDst, std::bitset<1> *branch, std::bitset<1> *memRead,
      std::bitset<1> *memToReg, std::bitset<2> *aluOp, std::bitset<1> *memWrite,
      std::bitset<1> *aluSrc, std::bitset<1> *regWrite);
  // the ALU Control unit
  void ALUControl(
      const std::bitset<2> *aluOp, const std::bitset<6> *funct,
      std::bitset<4> *aluControl);

private:
  // wires
  std::bitset<32> m_instruction; // InstructionMemory --> Instruction
  // misc.
  unsigned long long m_currCycle; // <-- tracks the # of cycles the CPU processed
};

#endif
