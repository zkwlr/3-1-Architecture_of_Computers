#include "SingleCycleCPU.hpp"

/******************************************************/
/* SingleCycleCPU::AND                                */
/*   - Perform logical AND on two BitWidth-bit values */
/*   - output = input0 & input1;                      */
/******************************************************/
template <size_t BitWidth>
void SingleCycleCPU::AND(
    const std::bitset<BitWidth> *input0, const std::bitset<BitWidth> *input1,
    std::bitset<BitWidth> *output)
{
  *output = (*input0) & (*input1); // input0 과 input1의 AND 연산을 수행하고 output에 저장
}

/********************************************/
/* SingleCycleCPU::Add                      */
/*   - Add two BitWidth-bit signed integers */
/*   - output = input0 + input1;            */
/********************************************/
template <size_t BitWidth>
void SingleCycleCPU::Add(
    const std::bitset<BitWidth> *input0, const std::bitset<BitWidth> *input1,
    std::bitset<BitWidth> *output)
{
  *output = (*input0).to_ulong() + (*input1).to_ulong(); // input0과 input1을 더한 값을 output에 저장
  // overflow를 처리하려면 std::bitset<BitWidth+1>에 저장한 후
  // output에 그 값을 다시 저장해 맨 윗자리를 없앤다. (계산결과는 틀림)
}

/***********************************************************************/
/* SingleCycleCPU::Mux                                                 */
/*   - Select one of the two BitWidth-bit inputs as the output         */
/*   - if (select == 0) { output = input0; } else { output = input1; } */
/***********************************************************************/
template <size_t BitWidth>
void SingleCycleCPU::Mux(
    const std::bitset<BitWidth> *input0, const std::bitset<BitWidth> *input1,
    const std::bitset<1> *select,
    std::bitset<BitWidth> *output)
{
  if (*select == 0b0)
  {                    // If select = 0
    *output = *input0; // output = input0
  }
  else
  {                    // If select = 1
    *output = *input1; // output = input1
  }
}

/***************************************************************************/
/* SingleCycleCPU::SignExtend                                              */
/*   - Expand an InputBitWidth-bit signed integer to an OutputBitWidth-bit */
/*     signed integer                                                      */
/*   - output = input;                                                     */
/***************************************************************************/
template <size_t InputBitWidth, size_t OutputBitWidth>
void SingleCycleCPU::SignExtend(
    const std::bitset<InputBitWidth> *input,
    std::bitset<OutputBitWidth> *output)
{
  int32_t outputa = static_cast<int32_t>(static_cast<int16_t>((*input).to_ulong()));
  *output = outputa;
  // ex)(N=8, M=16) 10000001 ->1111111110000001(9~16자리는 8번쨰 비트로, 1~8자리는 똑같이)
}

/*******************************************************************/
/* SingleCycleCPU::ShiftLeft2                                      */
/*   - Shift the BitWidth-bit signed integer to the left by 2 bits */
/*   - output = (input << 2);                                      */
/*******************************************************************/
template <size_t BitWidth>
void SingleCycleCPU::ShiftLeft2(
    const std::bitset<BitWidth> *input,
    std::bitset<BitWidth> *output)
{
  *output = (*input) << 2; // input을 2비트 left shift한 값을 output에 저장
}

/*****************************************************************/
/* SingleCycleCPU::ALU                                           */
/*   - Perform an arithmetic/logical operation on the two inputs */
/*   - Refer to the slides and textbook for the exact operation  */
/*****************************************************************/
void SingleCycleCPU::ALU(
    const std::bitset<32> *input0, const std::bitset<32> *input1,
    const std::bitset<4> *control,
    std::bitset<32> *output, std::bitset<1> *zero)
{
  if ((*control).to_string() == "0000") // AND
  {
    AND<32>(input0, input1, output);
  }
  else if ((*control).to_string() == "0001") // OR
  {
    *output = (*input0) | (*input1);
  }
  else if ((*control).to_string() == "0010") // add
  {
    Add<32>(input0, input1, output);
  }
  else if ((*control).to_string() == "0110") // subtract
  {
    *output = (*input0).to_ulong() - (*input1).to_ulong();
    if (*output == 0b0) // beq에서 rs값 = rt값이면 zero signal = 1 을 and gate에 보냄
                        // and gate에 도착한 branch signal도 1이면 branch 수행
    {
      *zero = 0b1;
    }
    else
    {
      *zero = 0b0;
    }
  }
  else if ((*control).to_string() == "0111") // set on less than
  {
    if ((*input0).to_ulong() < (*input1).to_ulong())
    {
      *output = 1;
    }
    else
    {
      *output = 0;
    }
  }
  else if ((*control).to_string() == "1100") // NOR
  {
    *output = ~((*input0) | (*input1));
  }
}

/*****************************************************************************************/
/* SingleCycleCPU::Control                                                               */
/*   - Produce appropriate control signals for the datapath w.r.t. the provided `opcode' */
/*****************************************************************************************/
void SingleCycleCPU::Control(
    const std::bitset<6> *opcode,
    std::bitset<1> *regDst, std::bitset<1> *branch, std::bitset<1> *memRead,
    std::bitset<1> *memToReg, std::bitset<2> *aluOp, std::bitset<1> *memWrite,
    std::bitset<1> *aluSrc, std::bitset<1> *regWrite)
{
  if (*opcode == 0b000000)
  { // R-type
    *regDst = 1;
    *branch = 0;
    *memRead = 0;
    *memToReg = 0;
    *aluOp = 0b10;
    *memWrite = 0;
    *aluSrc = 0;
    *regWrite = 1;
  }
  else if (*opcode == 0b100011)
  { // lw
    *regDst = 0;
    *branch = 0;
    *memRead = 1;
    *memToReg = 1;
    *aluOp = 0b00;
    *memWrite = 0;
    *aluSrc = 1;
    *regWrite = 1;
  }
  else if (*opcode == 0b101011)
  { // sw
    *regDst = 0;
    *branch = 0;
    *memRead = 0;
    *memToReg = 0;
    *aluOp = 0b00;
    *memWrite = 1;
    *aluSrc = 1;
    *regWrite = 0;
  }
  else if (*opcode == 0b000100)
  { // beq
    *regDst = 0;
    *branch = 1;
    *memRead = 0;
    *memToReg = 0;
    *aluOp = 0b01;
    *memWrite = 0;
    *aluSrc = 0;
    *regWrite = 0;
  }
  else if (*opcode == 0b001000)
  { // addi
    *regDst = 0;
    *branch = 0;
    *memRead = 0;
    *memToReg = 0;
    *aluOp = 0b00;
    *memWrite = 0;
    *aluSrc = 1;
    *regWrite = 1;
  }
  else if (*opcode == 0b001101)
  { // ori
    *regDst = 0;
    *branch = 0;
    *memRead = 0;
    *memToReg = 0;
    *aluOp = 0b00;
    *memWrite = 0;
    *aluSrc = 1;
    *regWrite = 1;
  }
  else if (*opcode == 0b001111)
  { // lui
    *regDst = 0;
    *branch = 0;
    *memRead = 0;
    *memToReg = 0;
    *aluOp = 0b00;
    *memWrite = 0;
    *aluSrc = 1;
    *regWrite = 1;
  }
  else if (*opcode == 0b000010)
  { // j
    *regDst = 0;
    *branch = 0;
    *memRead = 0;
    *memToReg = 0;
    *aluOp = 0b00;
    *memWrite = 0;
    *aluSrc = 0;
    *regWrite = 0;
  }
  else
  {
    *regDst = 0;
    *branch = 0;
    *memRead = 0;
    *memToReg = 0;
    *aluOp = 0b00;
    *memWrite = 0;
    *aluSrc = 0;
    *regWrite = 0;
    printf("ERROR : Invalid opcode\n");
  }
}

/***********************************************************************************************/
/* SingleCycleCPU::ALUControl                                                                  */
/*   - Produce the appropriate control signal for the ALU w.r.t. the given `ALUOp' and `funct' */
/***********************************************************************************************/
void SingleCycleCPU::ALUControl(
    const std::bitset<2> *aluOp, const std::bitset<6> *funct,
    std::bitset<4> *aluControl)
{
  if (*aluOp == 0b00)
  { // lw, sw
    *aluControl = 0b0010;
  }
  else if (*aluOp == 0b01)
  { // beq
    *aluControl = 0b0110;
  }
  else if (*aluOp == 0b10)
  { // R-type
    if (*funct == 0b100000)
    { // add
      *aluControl = 0b0010;
    }
    else if (*funct == 0b100010)
    { // sub
      *aluControl = 0b0110;
    }
    else if (*funct == 0b100100)
    { // and
      *aluControl = 0b0000;
    }
    else if (*funct == 0b100101)
    { // or
      *aluControl = 0b0001;
    }
    else if (*funct == 0b101010)
    { // set on less than
      *aluControl = 0b0111;
    }
    else if (*funct == 0b100111) // <-- 이부분 제출전 구현 안됨(명세서 표에 없고 테스트 파일에도 없었는데 감점되나?)
    {                            // nor
      *aluControl = 0b1100;
    }
    else
    {
      printf("ERROR : Invalid funct\n");
    }
  }
  else
  {
    printf("ERROR : Invalid aluOp\n");
  }
}

/*****************************************************************/
/* SingleCycleCPU::AdvanceCycle                                  */
/*   - Execute a single MIPS instruction in a single clock cycle */
/*****************************************************************/
void SingleCycleCPU::advanceCycle()
{
  /* DO NOT CHANGE THE FOLLOWING TWO LINES */
  m_currCycle++;
  printf("INFO: Cycle %llu: Executing the instruction located at 0x%08lx\n", m_currCycle, m_PC.to_ulong());

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
  m_instruction = *readinstData;                                         // readData port에서 wire로 instruction code를 보냄
  // m_PC = m_PC.to_ulong() + 4; //PC = PC + 4
  std::bitset<32> four(4);
  Add<32>(&m_PC, &four, &m_PC); // PC = PC + 4

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
  ShiftLeft2<32>(signExtendedImmediate, signExtendedImmediate);

  // ALU for Branch
  std::bitset<32> *branchaluinput1 = &m_PC;                 // PC + 4
  std::bitset<32> *branchaluinput2 = signExtendedImmediate; // offset*4
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
  // regWrite 신호에 따라 register에 write back할지 결정
  m_registerFile->access(ReadRegister1, ReadRegister2, writeRegister, writeBackData, regWrite, readData1, readData2);

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
  delete aluControl;
  delete aluinput2;
  delete aluResult;
  delete zero;
  delete branchaluResult;
  delete PCSrc;
  delete readData;
}
