#include "SingleCycleCPU.hpp"

/*****************************************************************/
/* SingleCycleCPU::advanceCycle                                  */
/*   - Execute a single MIPS instruction in a single clock cycle */
/*****************************************************************/
void SingleCycleCPU::advanceCycle()
{
  /* DO NOT CHANGE THE FOLLOWING LINE */
  CPU::advanceCycle();

  // 시작 PC : m_PC(initialPC),
  // Register file 객체 : m_registerFile(new RegisterFile(regFileName)),
  // Instruction Memory 객체 : m_instMemory(new Memory(Memory::LittleEndian, instMemFileName)),
  // Data Memory 객체 : m_dataMemory(new Memory(Memory::LittleEndian, dataMemFileName)),

  // Instruction Fetch용 wire(datapath), port 배치
  std::bitset<32> *writeBackData = new std::bitset<32>; // register에 write back할 data
  std::bitset<32> *readinstData = new std::bitset<32>;

  // signal 배치
  std::bitset<1> *regDst = new std::bitset<1>;
  std::bitset<1> *branch = new std::bitset<1>;
  std::bitset<1> *memRead = new std::bitset<1>;
  std::bitset<1> *memToReg = new std::bitset<1>;
  std::bitset<2> *aluOp = new std::bitset<2>;
  std::bitset<1> *memWrite = new std::bitset<1>;
  std::bitset<1> *aluSrc = new std::bitset<1>;
  std::bitset<1> *regWrite = new std::bitset<1>;

  // 1. Fetch
  *memRead = 0b1;                                                        // instruction memory에서 읽어야 하므로 memRead 신호는 1이다.
  m_instMemory->access(&m_PC, nullptr, memRead, memWrite, readinstData); // readData에 현재 PC가 가리키는 instruction 저장
  std::bitset<32> m_instruction = *readinstData;                         // readData port에서 wire로 instruction code를 보냄
  // m_PC = m_PC.to_ulong() + 4; //PC = PC + 4
  std::bitset<32> four(4);
  std::bitset<32> temp(0);      // PC 주소 임시 저장 변수
  Add<32>(&m_PC, &four, &temp); // PC = PC + 4
  m_PC = temp;

  // 2. Decode - Parse the fetched instruction
  std::bitset<6> opcode = m_instruction.to_ulong() >> 26;         // instruction code의 상위 6bit만 남게 left shift
  std::bitset<5> rs = (m_instruction.to_ulong() >> 21) & 0b11111; // 0b11111 = and 연산통해 하위 5bit의 값만 가져옴
  std::bitset<5> rt = (m_instruction.to_ulong() >> 16) & 0b11111;
  std::bitset<5> rd = (m_instruction.to_ulong() >> 11) & 0b11111;
  std::bitset<6> funct = m_instruction.to_ulong() & 0b111111;
  std::bitset<16> immediate = m_instruction.to_ulong() & 0xFFFF; // 하위 16bit만 가져옴

  // Decode - Set control signals by opcode
  Control(&opcode, regDst, branch, memRead, memToReg, aluOp, memWrite, aluSrc, regWrite);

  // 3. EX - Register File
  std::bitset<5> *ReadRegister1 = &rs; // wire(rs) -> port(readRegister1)
  std::bitset<5> *ReadRegister2 = &rt;
  std::bitset<5> *writeRegister = new std::bitset<5>;
  std::bitset<32> *readData1 = new std::bitset<32>;
  std::bitset<32> *readData2 = new std::bitset<32>;
  Mux<5>(&rt, &rd, regDst, writeRegister); // WriteRegister port 앞의 MUX: regDst가 1이면 writeRegister = rd, 0이면 writeRegister = rt
  // readData1, readData2에 현재 register에 저장된 값 저장
  m_registerFile->access(ReadRegister1, ReadRegister2, writeRegister, writeBackData, regWrite, readData1, readData2);

  // Sign Extend
  std::bitset<32> *signExtendedImmediate = new std::bitset<32>;
  SignExtend<16, 32>(&immediate, signExtendedImmediate);

  // ALU Control
  std::bitset<4> *aluControl = new std::bitset<4>;
  ALUControl(aluOp, &funct, aluControl); // ALU가 어떤 연산을 해야할지 결정하는 signal 생성

  // ALU
  std::bitset<32> *aluinput1 = readData1;
  std::bitset<32> *aluinput2 = new std::bitset<32>;
  std::bitset<32> *aluResult = new std::bitset<32>;
  std::bitset<1> *zero = new std::bitset<1>;

  // aluinput2 port 앞의 MUX: aluSrc가 1이면 aluinput2 = signExtendedImmediate, 0이면 aluinput2 = readData2
  Mux<32>(readData2, signExtendedImmediate, aluSrc, aluinput2);
  ALU(aluinput1, aluinput2, aluControl, aluResult, zero); // ALU 연산 수행

  // branch 공식은 (PC + 4) + offset*4 이므로 sign-extended 값을 shiftleft2
  std::bitset<32> *sl2signExtendedImmediate = new std::bitset<32>; // sign-extended 값을 shiftleft2한 값
  ShiftLeft2<32>(signExtendedImmediate, sl2signExtendedImmediate);

  // ALU for Branch
  std::bitset<32> *branchaluinput1 = &m_PC;                    // PC + 4
  std::bitset<32> *branchaluinput2 = sl2signExtendedImmediate; // offset*4
  std::bitset<32> *branchaluResult = new std::bitset<32>;
  Add<32>(branchaluinput1, branchaluinput2, branchaluResult); // PC + 4 + offset*4

  // branch를 위한 and gate
  std::bitset<1> *PCSrc = new std::bitset<1>; // PCSrc = 1이면 branch, 0이면 PC + 4
  AND<1>(branch, zero, PCSrc);

  // PC 값 결정을 위한 MUX : PCSrc = 1 이면 PC+4+offset*4, 0이면 PC+4
  Mux<32>(branchaluinput1, branchaluResult, PCSrc, &m_PC);

  // 4. Data Memory
  std::bitset<32> *address = aluResult;            // aluResult port ->(wire)-> address port
  std::bitset<32> *m_writeData = readData2;        // readData2 port ->(wire)-> m_writeData port
  std::bitset<32> *readData = new std::bitset<32>; // output port

  // memRead, memWrite의 상태에 따라 read할지, write할지, 접근 안할건지 결정
  // (Data memory 앞에 있는 MUX에서 WB은 언제나 일어남, regWrite 신호에 따라 WriteRegister에 writeData를 write할지 결정)
  m_dataMemory->access(address, m_writeData, memRead, memWrite, readData);

  // 5. Write Back
  // WB할 wire은 위에 미리 정의함 (writeBackData)
  // Data memory 앞의 MUX에서 WB할 데이터 결정
  Mux<32>(aluResult, readData, memToReg, writeBackData); // MemtoReg = 1 이면 readData, 0이면 aluResult
  // lw rt:$0, offset(rs), add $0, $0, $0 등의 경우
  // WB할 레지스터(WriteRegister = rt or rd)가 $0(Zero reg)이면 WB 명령을 prevent한다.
  if (writeRegister->to_ulong() != 0)
  {
    // regWrite 신호에 따라 register에 write back할지 결정
    m_registerFile->access(ReadRegister1, ReadRegister2, writeRegister, writeBackData, regWrite, readData1, readData2);
  }

  // 동적으로 할당된 메모리 모두 삭제
  delete writeBackData;
  delete readinstData;
  delete regDst;
  delete branch;
  delete memRead;
  delete memToReg;
  delete aluOp;
  delete memWrite;
  delete aluSrc;
  delete regWrite;
  delete writeRegister;
  delete readData1;
  delete readData2;
  delete signExtendedImmediate;
  delete sl2signExtendedImmediate;
  delete aluControl;
  delete aluinput2;
  delete aluResult;
  delete zero;
  delete branchaluResult;
  delete PCSrc;
  delete readData;
}
