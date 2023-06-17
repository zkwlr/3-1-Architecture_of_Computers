#include "PipelinedCPU.hpp"

void PipelinedCPU::InstructionFetch()
{
  // 시작 PC : m_PC(initialPC),
  // Register file 객체 : m_registerFile(new RegisterFile(regFileName)),
  // Instruction Memory 객체 : m_instMemory(new Memory(Memory::LittleEndian, instMemFileName)),
  // Data Memory 객체 : m_dataMemory(new Memory(Memory::LittleEndian, dataMemFileName)),

  // Update PC
  std::bitset<32> four(4);
  std::bitset<32> temp(0);      // PC 주소 임시 저장 변수
  Add<32>(&m_PC, &four, &temp); // 이번 cycle에 읽을 PC = PC + 4 (전 clock의 pcPlus4 값)
  m_PC = temp;

  // Instruction Fetch용 wire(datapath), port 배치, IF의 second half clock
  // Fetch instruction from memory
  std::bitset<1> memRead(1);
  std::bitset<1> memWrite(0);
  // PC+4 값이 아니라 현재 PC(clock 전반부에 이미 4 증가됨) 주소의 instruction 읽는다.
  m_instMemory->access(&m_PC, nullptr, &memRead, &memWrite, &m_latch_IF_ID.instr); // readData에 현재 PC가 가리키는 instruction 저장

  // Set IF/ID latch
  // Figure 4.51의 IF 부분의 adder 구현 (branch target을 계산할 때 쓸 pc+4 주소를 latch에 저장)
  // 현재 PC를 또 4만큼 늘린게 아니라 latch의 pcPlus4 가 (현재 PC)+4 인것이다.
  // 즉, m_PC의 값은 변함 없음
  Add<32>(&m_PC, &four, &m_latch_IF_ID.pcPlus4);
}

void PipelinedCPU::InstructionDecode()
{
  // 2. Decode - Parse the fetched instruction
  std::bitset<6> opcode = m_latch_IF_ID.instr.to_ulong() >> 26;         // instruction code의 상위 6bit만 남게 left shift
  std::bitset<5> rs = (m_latch_IF_ID.instr.to_ulong() >> 21) & 0b11111; // 0b11111 = and 연산통해 하위 5bit의 값만 가져옴
  std::bitset<5> rt = (m_latch_IF_ID.instr.to_ulong() >> 16) & 0b11111;
  std::bitset<5> rd = (m_latch_IF_ID.instr.to_ulong() >> 11) & 0b11111;
  std::bitset<16> immediate = m_latch_IF_ID.instr.to_ulong() & 0xFFFF; // 하위 16bit만 가져옴

  // Decode - Set control signals by opcode, and Set ID/EX latch
  Control(&opcode, &m_latch_ID_EX.ctrlEXRegDst, &m_latch_ID_EX.ctrlMEMBranch, &m_latch_ID_EX.ctrlMEMMemRead,
          &m_latch_ID_EX.ctrlWBMemToReg, &m_latch_ID_EX.ctrlEXALUOp, &m_latch_ID_EX.ctrlMEMMemWrite,
          &m_latch_ID_EX.ctrlEXALUSrc, &m_latch_ID_EX.ctrlWBRegWrite);
  m_latch_ID_EX.pcPlus4 = m_latch_IF_ID.pcPlus4;
  // readData1, readData2에 현재 register에 저장된 값 저장, Writedata는 하지 않으므로 관련 port와 signal은 nullptr
  m_registerFile->access(&rs, &rt, nullptr, nullptr, nullptr, &m_latch_ID_EX.readData1, &m_latch_ID_EX.readData2);
  SignExtend<16, 32>(&immediate, &m_latch_ID_EX.immediate);
  m_latch_ID_EX.instr_20_16 = rt; // rt값과 rd값중 뭐가 Writereg인지 결정하는 Mux가 EX stage에 있으므로 넘겨줘야함
  m_latch_ID_EX.instr_15_11 = rd; // WB 단계에서 rd(Write Register)에 WB 해줘야 하므로 latch에 저장해서 다음으로 넘김
}

void PipelinedCPU::Execute()
{
  // 3. EX : Figure 4.51의 아랫 부분부터 구현
  Mux<5>(&m_latch_ID_EX.instr_20_16, &m_latch_ID_EX.instr_15_11,
         &m_latch_ID_EX.ctrlEXRegDst, &m_latch_EX_MEM.rd); // EX stage의 맨 아래 MUX: regDst가 1이면 rd = rd, 0이면 rd = rt
  std::bitset<32> aluinput2;
  Mux<32>(&m_latch_ID_EX.readData2, &m_latch_ID_EX.immediate,
          &m_latch_ID_EX.ctrlEXALUSrc, &aluinput2); // EX stage의 두번째 MUX: ALUSrc가 1이면 aluinput2 = immediate, 0이면 aluinput2 = readData2
  // branch 공식은 (PC + 4) + offset*4 이므로 sign-extended 값을 shiftleft2
  std::bitset<32> shiftleft2Immediate;
  ShiftLeft2<32>(&m_latch_ID_EX.immediate, &shiftleft2Immediate);
  std::bitset<6> funct = m_latch_ID_EX.immediate.to_ulong() & 0b111111; // funct : [5-0]
  std::bitset<4> aluControl;
  ALUControl(&m_latch_ID_EX.ctrlEXALUOp, &funct, &aluControl); // ALU가 어떤 연산을 해야할지 결정하는 signal 생성
  // ALU unit
  ALU(&m_latch_ID_EX.readData1, &aluinput2, &aluControl,
      &m_latch_EX_MEM.aluResult, &m_latch_EX_MEM.aluZero); // ALU 연산 결과와 zero signal 생성
  // ADD unit
  Add<32>(&m_latch_ID_EX.pcPlus4, &shiftleft2Immediate, &m_latch_EX_MEM.branchTarget); // branch 했을 때 변경될 branch target 주소 계산
  // Set remaining EX/MEM latch
  m_latch_EX_MEM.readData2 = m_latch_ID_EX.readData2;
  m_latch_EX_MEM.ctrlMEMBranch = m_latch_ID_EX.ctrlMEMBranch;
  m_latch_EX_MEM.ctrlMEMMemRead = m_latch_ID_EX.ctrlMEMMemRead;
  m_latch_EX_MEM.ctrlMEMMemWrite = m_latch_ID_EX.ctrlMEMMemWrite;
  m_latch_EX_MEM.ctrlWBRegWrite = m_latch_ID_EX.ctrlWBRegWrite;
  m_latch_EX_MEM.ctrlWBMemToReg = m_latch_ID_EX.ctrlWBMemToReg;
}

void PipelinedCPU::MemoryAccess()
{
  // 4. MemoryAccess
  // branch를 위한 and gate
  std::bitset<1> PCSrc;                                                   // PCSrc = 1이면 branch, 0이면 PC + 4
  AND<1>(&m_latch_EX_MEM.ctrlMEMBranch, &m_latch_EX_MEM.aluZero, &PCSrc); // PCSrc signal 생성
  // PC 값 결정을 위한 MUX : PCSrc = 1 이면 PC+4+offset*4, 0이면 PC+4
  // 3번 과제에선 branch 명령어 다음에 bubble 2개가 존재하므로 control hazard가 없다.
  // 즉 같은 clock의 MEM stage에서 MUX를 통해 PC를 결정하고 IF에 있는 PC로 보낸다.
  // 이미 branchtarget = PC + 4 + offset*4인데, 그 값을 그대로 m_PC에 넘겨주면
  // IF stage 전반에서 +4가 또 이루어지므로, branchtarget에 -4를 해줘야 제대로 분기가 작동한다.
  std::bitset<32> realbranchtarget = m_latch_EX_MEM.branchTarget.to_ulong() - 0x04;
  Mux<32>(&m_PC, &realbranchtarget, &PCSrc, &m_PC);
  // Data memory access
  // memRead, memWrite의 상태에 따라 read할지, write할지, 접근 안할건지 결정
  // (regWrite 신호에 따라 WriteRegister에 writeData를 write할지 결정)
  m_dataMemory->access(&m_latch_EX_MEM.aluResult, &m_latch_EX_MEM.readData2, &m_latch_EX_MEM.ctrlMEMMemRead,
                       &m_latch_EX_MEM.ctrlMEMMemWrite, &m_latch_MEM_WB.readData);
  // Set remaining MEM/WB latch
  m_latch_MEM_WB.aluResult = m_latch_EX_MEM.aluResult;
  m_latch_MEM_WB.rd = m_latch_EX_MEM.rd;
  m_latch_MEM_WB.ctrlWBRegWrite = m_latch_EX_MEM.ctrlWBRegWrite;
  m_latch_MEM_WB.ctrlWBMemToReg = m_latch_EX_MEM.ctrlWBMemToReg;
}

void PipelinedCPU::WriteBack()
{
  // 5. WriteBack
  // Data memory 앞의 MUX에서 WB할 데이터 결정
  std::bitset<32> writeBackData; // RegisterFile로 WB 위해 보낼 데이터
  Mux<32>(&m_latch_MEM_WB.aluResult, &m_latch_MEM_WB.readData,
          &m_latch_MEM_WB.ctrlWBMemToReg, &writeBackData); // MemtoReg = 1 이면 readData, 0이면 aluResult
  // lw rt:$0, offset(rs), add $0, $0, $0 등의 경우
  // WB할 레지스터(WriteRegister = rt or rd)가 $0(Zero reg)이면 WB 명령을 prevent한다.
  if (m_latch_MEM_WB.rd != 0)
  { // regWrite 신호에 따라 register에 write back할지 결정
    m_registerFile->access(nullptr, nullptr, &m_latch_MEM_WB.rd, &writeBackData,
                           &m_latch_MEM_WB.ctrlWBRegWrite, nullptr, nullptr);
  }
}
