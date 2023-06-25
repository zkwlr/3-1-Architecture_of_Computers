#include "PipelinedCPU.hpp"

void PipelinedCPU::InstructionFetch()
{
  // 시작 PC : m_PC(initialPC),
  // Register file 객체 : m_registerFile(new RegisterFile(regFileName)),
  // Instruction Memory 객체 : m_instMemory(new Memory(Memory::LittleEndian, instMemFileName)),
  // Data Memory 객체 : m_dataMemory(new Memory(Memory::LittleEndian, dataMemFileName)),

  // Update PC
  std::bitset<32> four(4);
  std::bitset<32> temp(0);         // PC 주소 임시 저장 변수
  std::bitset<32> nop_m_PC = m_PC; // PCWrite가 0일 때, 즉 PC값 업데이트를 하면 안될 때 사용할 변수
  Add<32>(&m_PC, &four, &temp);    // 이번 cycle에 읽을 PC = PC + 4 (전 clock의 pcPlus4 값)
  m_PC = temp;
  // PC 값 결정을 위한 MUX : PCSrc = 1 이면 PC+4+offset*4, 0이면 PC+4
  CPU::Mux<32>(&m_PC, &m_MEM_to_IF_branchTarget, &m_MEM_to_IF_PCSrc, &m_PC);

  // load-use data hazard를 위한 Mux
  // PCWrite = 0 이면 PC 값 업데이트 안함, 1이면 PC+4 or PC+4+offset*4로 다음 명령 처리
  CPU::Mux<32>(&nop_m_PC, &m_PC, &m_HazDetUnit_to_IF_PCWrite, &m_PC);

  // Instruction Fetch용 wire(datapath), port 배치, IF의 second half clock
  // Fetch instruction from memory
  std::bitset<1> memRead(1);
  std::bitset<1> memWrite(0);
  // Set IF/ID latch
  // load_use data hazard 방지를 위한 if
  // IF/IDWrite = 1 이면 IF/ID latch를 업데이트, 0이면 업데이트 안하고 nop(bubble) 실행
  if (m_HazDetUnit_to_IF_IFIDWrite == 1)
  {
    // PC+4 값이 아니라 현재 PC(clock 전반부에 이미 4 증가됨) 주소의 instruction 읽는다.
    m_instMemory->access(&m_PC, nullptr, &memRead, &memWrite, &m_latch_IF_ID.instr);
    // Figure 4.51의 IF 부분의 adder 구현 (branch target을 계산할 때 쓸 pc+4 주소를 latch에 저장)
    Add<32>(&m_PC, &four, &m_latch_IF_ID.pcPlus4);
  }
}

void PipelinedCPU::InstructionDecode()
{
  // 2. Decode - Parse the fetched instruction
  std::bitset<6> opcode = m_latch_IF_ID.instr.to_ulong() >> 26;         // instruction code의 상위 6bit만 남게 left shift
  std::bitset<5> rs = (m_latch_IF_ID.instr.to_ulong() >> 21) & 0b11111; // 0b11111 = and 연산통해 하위 5bit의 값만 가져옴
  std::bitset<5> rt = (m_latch_IF_ID.instr.to_ulong() >> 16) & 0b11111;
  std::bitset<5> rd = (m_latch_IF_ID.instr.to_ulong() >> 11) & 0b11111;
  std::bitset<16> immediate = m_latch_IF_ID.instr.to_ulong() & 0xFFFF; // 하위 16bit만 가져옴
  // Hazard detection unit 생성, PCWrite, IF/IDWrite, ctrlSelect signal 생성
  std::bitset<1> ctrlSelect = 0;
  HazardDetectionUnit(&rs, &rt, &m_EX_to_HazDetUnit_memRead, &m_EX_to_HazDetUnit_rt,
                      &m_HazDetUnit_to_IF_PCWrite, &m_HazDetUnit_to_IF_IFIDWrite, &ctrlSelect);

  // Decode - Set control signals by opcode, and Set ID/EX latch
  // FIXME control unit 앞의 mux 구현: ctrlSelect에 따라 if ctrlselect==1 이면 for(std::bitset<n> i : m_latch_ID_EX)  i = 0으로 넣는다.
  // 아니면 mux 사용
  Control(&opcode, &m_latch_ID_EX.ctrlEXRegDst, &m_latch_ID_EX.ctrlMEMBranch, &m_latch_ID_EX.ctrlMEMMemRead,
          &m_latch_ID_EX.ctrlWBMemToReg, &m_latch_ID_EX.ctrlEXALUOp, &m_latch_ID_EX.ctrlMEMMemWrite,
          &m_latch_ID_EX.ctrlEXALUSrc, &m_latch_ID_EX.ctrlWBRegWrite);
  // Hazard detection unit의 ctrlSelect = 1 이면 signal 전부 0으로, 0이면 만들어진 signal 그대로 보냄
  std::bitset<1> zero(0);
  std::bitset<2> zero_2bit(0);
  CPU::Mux<1>(&m_latch_ID_EX.ctrlEXALUSrc, &zero, &ctrlSelect, &m_latch_ID_EX.ctrlEXALUSrc);
  CPU::Mux<2>(&m_latch_ID_EX.ctrlEXALUOp, &zero_2bit, &ctrlSelect, &m_latch_ID_EX.ctrlEXALUOp);
  CPU::Mux<1>(&m_latch_ID_EX.ctrlEXRegDst, &zero, &ctrlSelect, &m_latch_ID_EX.ctrlEXRegDst);
  CPU::Mux<1>(&m_latch_ID_EX.ctrlMEMBranch, &zero, &ctrlSelect, &m_latch_ID_EX.ctrlMEMBranch);
  CPU::Mux<1>(&m_latch_ID_EX.ctrlMEMMemRead, &zero, &ctrlSelect, &m_latch_ID_EX.ctrlMEMMemRead);
  CPU::Mux<1>(&m_latch_ID_EX.ctrlMEMMemWrite, &zero, &ctrlSelect, &m_latch_ID_EX.ctrlMEMMemWrite);
  CPU::Mux<1>(&m_latch_ID_EX.ctrlWBRegWrite, &zero, &ctrlSelect, &m_latch_ID_EX.ctrlWBRegWrite);
  CPU::Mux<1>(&m_latch_ID_EX.ctrlWBMemToReg, &zero, &ctrlSelect, &m_latch_ID_EX.ctrlWBMemToReg);

  m_latch_ID_EX.pcPlus4 = m_latch_IF_ID.pcPlus4;
  // readData1, readData2에 현재 register에 저장된 값 저장, Writedata는 하지 않으므로 관련 port와 signal은 nullptr
  m_registerFile->access(&rs, &rt, nullptr, nullptr, nullptr, &m_latch_ID_EX.readData1, &m_latch_ID_EX.readData2);
  SignExtend<16, 32>(&immediate, &m_latch_ID_EX.immediate);
  m_latch_ID_EX.instr_25_21 = rs; // IF/ID stage를 실행중인 명령(data forwarding을 받을 명령)의 rs값을 forwarding unit이 받아야 하므로 latch에 추가로 저장
  m_latch_ID_EX.instr_20_16 = rt; // rt값과 rd값중 뭐가 Writereg인지 결정하는 Mux가 EX stage에 있으므로 넘겨줘야함, 또한 rs와 같은 이유로 forwarding unit에 넘겨줘야함
  m_latch_ID_EX.instr_15_11 = rd; // WB 단계에서 rd(Write Register)에 WB 해줘야 하므로 latch에 저장해서 다음으로 넘김
}

void PipelinedCPU::Execute()
{
  // 3. EX : Figure 4.51의 아랫 부분부터 구현
  // ID stage에 있는 Hazard detection unit에 값을 전달하기 위해 연결된 포트에 ID/EX.MemRead 값과 ID/EX.rt 값 보냄
  m_EX_to_HazDetUnit_memRead = m_latch_ID_EX.ctrlMEMMemRead;
  m_EX_to_HazDetUnit_rt = m_latch_ID_EX.instr_20_16;
  // 3-to-1 MUX를 위한 forwarding signal 생성, MUX에서 결정된 rs, rt 값 저장할 변수 생성
  std::bitset<2> forwardA;
  std::bitset<2> forwardB;
  std::bitset<32> forwarded_rsValue;
  std::bitset<32> forwarded_rtValue;
  // Data forwarding unit 생성, forwarding signal 생성
  ForwardingUnit(&m_latch_ID_EX.instr_25_21, &m_latch_ID_EX.instr_20_16, &m_latch_EX_MEM.ctrlWBRegWrite,
                 &m_latch_EX_MEM.rd, &m_latch_MEM_WB.ctrlWBRegWrite, &m_latch_MEM_WB.rd, &forwardA, &forwardB);
  // ID/EX의 rs Data forwarding을 위한 3-to-1 MUX, forwardA signal 받음
  Mux<32>(&m_latch_ID_EX.readData1, &m_WB_to_FwdUnit_rdValue, &m_MEM_to_FwdUnit_rdValue, &forwardA, &forwarded_rsValue);
  // ID/EX의 rt Data forwarding을 위한 3-to-1 MUX, forwardB signal 받음
  Mux<32>(&m_latch_ID_EX.readData2, &m_WB_to_FwdUnit_rdValue, &m_MEM_to_FwdUnit_rdValue, &forwardB, &forwarded_rtValue);
  CPU::Mux<5>(&m_latch_ID_EX.instr_20_16, &m_latch_ID_EX.instr_15_11,
              &m_latch_ID_EX.ctrlEXRegDst, &m_latch_EX_MEM.rd); // EX stage의 맨 아래 MUX: regDst가 1이면 rd = rd, 0이면 rd = rt
  std::bitset<32> aluinput2;
  CPU::Mux<32>(&forwarded_rtValue, &m_latch_ID_EX.immediate,
               &m_latch_ID_EX.ctrlEXALUSrc, &aluinput2); // EX stage의 두번째 MUX: ALUSrc가 1이면 aluinput2 = immediate, 0이면 aluinput2 = forwardedrt
  // branch 공식은 (PC + 4) + offset*4 이므로 sign-extended 값을 shiftleft2
  std::bitset<32> shiftleft2Immediate;
  ShiftLeft2<32>(&m_latch_ID_EX.immediate, &shiftleft2Immediate);
  std::bitset<6> funct = m_latch_ID_EX.immediate.to_ulong() & 0b111111; // funct : [5-0]
  std::bitset<4> aluControl;
  ALUControl(&m_latch_ID_EX.ctrlEXALUOp, &funct, &aluControl); // ALU가 어떤 연산을 해야할지 결정하는 signal 생성
  // ALU unit
  ALU(&forwarded_rsValue, &aluinput2, &aluControl,
      &m_latch_EX_MEM.aluResult, &m_latch_EX_MEM.aluZero); // ALU 연산 결과와 zero signal 생성
  // ADD unit
  Add<32>(&m_latch_ID_EX.pcPlus4, &shiftleft2Immediate, &m_latch_EX_MEM.branchTarget); // branch 했을 때 변경될 branch target 주소 계산
  // Set remaining EX/MEM latch
  m_latch_EX_MEM.readData2 = forwarded_rtValue; // 3-to-1 MUX에서 결정된 값이 이제 readData2 이므로 결정된 값을 EX/MEM latch로 넘겨준다.
  m_latch_EX_MEM.ctrlMEMBranch = m_latch_ID_EX.ctrlMEMBranch;
  m_latch_EX_MEM.ctrlMEMMemRead = m_latch_ID_EX.ctrlMEMMemRead;
  m_latch_EX_MEM.ctrlMEMMemWrite = m_latch_ID_EX.ctrlMEMMemWrite;
  m_latch_EX_MEM.ctrlWBRegWrite = m_latch_ID_EX.ctrlWBRegWrite;
  m_latch_EX_MEM.ctrlWBMemToReg = m_latch_ID_EX.ctrlWBMemToReg;
}

void PipelinedCPU::MemoryAccess()
{
  // 4. MemoryAccess
  // MEM 하기전 EX/MEM latch에 있는 regWrite, rd, rdValue를 forwarding unit에 넘겨준다.
  m_MEM_to_FwdUnit_regWrite = m_latch_EX_MEM.ctrlWBRegWrite; // 일단 두번째로 실행된 명령이 레지스터에 값을 쓰는 명령이어야 하고,
  // MEM/WB stage에 있는 두번쨰로 실행된 명령의 rd와 rd에 저장되어 있는 값을 조건이 만족하면 ID/EX stage를 실행중인 명령의 rs나 rt의 값으로 준다.
  m_MEM_to_FwdUnit_rd = m_latch_EX_MEM.rd;
  m_MEM_to_FwdUnit_rdValue = m_latch_EX_MEM.aluResult;

  // branch를 위한 and gate
  // PCSrc = 1이면 branch, 0이면 PC + 4
  // MEM 하기전 EX/MEM latch에 있는 PCSrc, branchTarget 값을 IF stage에 넘겨줘야 IF에서 MUX를 구현할 수 있다.
  AND<1>(&m_latch_EX_MEM.ctrlMEMBranch, &m_latch_EX_MEM.aluZero, &m_MEM_to_IF_PCSrc); // PCSrc signal 생성
  m_MEM_to_IF_branchTarget = m_latch_EX_MEM.branchTarget;
  // 즉 같은 clock의 MEM stage에서 MUX를 통해 PC를 결정하고 IF에 있는 PC로 보낸다.
  // 이미 branchtarget = PC + 4 + offset*4인데, 그 값을 그대로 m_PC에 넘겨주면
  // IF stage 전반에서 +4가 또 이루어지므로, branchtarget에 -4를 해줘야 제대로 분기가 작동한다.
  // 과제 4에선 m_MEM_to_IF_PCSrc와 branchTarget 포트가 생겼으므로 Mux의 위치를 if로 옮길 수 있다.
  // std::bitset<32>realbranchtarget = m_latch_EX_MEM.branchTarget.to_ulong() - 0x04;

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
  // WB 하기전 MEM/WB latch에 있는 regWrite, rd, rdValue를 forwarding unit에 넘겨준다.
  // WB stage에서 미리 하지 않으면 다음에 실행될 MEM stage가 MEM/WB latch 값을 덮어쓰게 되어 이전 Data가 사라지기 때문이다.
  // 이 과정을 MEM stage 다음에 실행될 가장 최근 명령의 Ex stage에서 한다면 이미 덮어쓰어진 EX/MEM latch의 rd 값이 전달되므로
  // 제대로 된 forwarding이 불가능
  m_WB_to_FwdUnit_regWrite = m_latch_MEM_WB.ctrlWBRegWrite; // 일단 가장 먼저 실행된 명령이 레지스터에 값을 쓰는 명령이어야 하고,

  // Data memory 앞의 MUX에서 WB할 데이터 결정
  std::bitset<32> writeBackData; // RegisterFile로 WB 위해 보낼 데이터
  CPU::Mux<32>(&m_latch_MEM_WB.aluResult, &m_latch_MEM_WB.readData,
               &m_latch_MEM_WB.ctrlWBMemToReg, &writeBackData); // MemtoReg = 1 이면 readData, 0이면 aluResult

  // MEM/WB stage에 있는 rd는 forwarding이 필요한지 조건을 알기위해 필요한 포트
  // rdValue는 reg에 WB할 값을 forwarding으로 EX에 바로 넘겨주기 위한 포트
  m_WB_to_FwdUnit_rd = m_latch_MEM_WB.rd;
  m_WB_to_FwdUnit_rdValue = writeBackData;
  // regWrite 신호에 따라 register에 write back할지 결정
  m_registerFile->access(nullptr, nullptr, &m_latch_MEM_WB.rd, &writeBackData,
                         &m_latch_MEM_WB.ctrlWBRegWrite, nullptr, nullptr);
}

void PipelinedCPU::ForwardingUnit(
    const std::bitset<5> *ID_EX_rs, const std::bitset<5> *ID_EX_rt,
    const std::bitset<1> *EX_MEM_regWrite, const std::bitset<5> *EX_MEM_rd,
    const std::bitset<1> *MEM_WB_regWrite, const std::bitset<5> *MEM_WB_rd,
    std::bitset<2> *forwardA, std::bitset<2> *forwardB)
{
  // Forward A
  if ((m_WB_to_FwdUnit_regWrite == 1 && m_WB_to_FwdUnit_rd != 0) // MEM/WB의 rd를 ID/EX의 rs로 forwarding하는 경우
      && !(m_MEM_to_FwdUnit_regWrite == 1 && m_MEM_to_FwdUnit_rd != 0 && m_MEM_to_FwdUnit_rd == *ID_EX_rs) && m_WB_to_FwdUnit_rd == *ID_EX_rs)
  {
    *forwardA = 0b01;
  }
  else if (m_MEM_to_FwdUnit_regWrite == 1 && m_MEM_to_FwdUnit_rd != 0 && m_MEM_to_FwdUnit_rd == *ID_EX_rs)
  { // EX/MEM의 rd를 ID/EX의 rs로 forwarding하는 경우
    *forwardA = 0b10;
  }
  else
  { // ID/EX의 rs 값를 그대로 쓰는 경우(forwarding 안하는 경우)
    *forwardA = 0b00;
  }

  // Forward B
  if ((m_WB_to_FwdUnit_regWrite == 1 && m_WB_to_FwdUnit_rd != 0) // MEM/WB의 rd를 ID/EX의 rt로 forwarding하는 경우
      && !(m_MEM_to_FwdUnit_regWrite == 1 && m_MEM_to_FwdUnit_rd != 0 && m_MEM_to_FwdUnit_rd == *ID_EX_rt) && m_WB_to_FwdUnit_rd == *ID_EX_rt)
  {
    *forwardB = 0b01;
  }
  else if (m_MEM_to_FwdUnit_regWrite == 1 && m_MEM_to_FwdUnit_rd != 0 && m_MEM_to_FwdUnit_rd == *ID_EX_rt)
  { // EX/MEM의 rd를 ID/EX의 rt로 forwarding하는 경우
    *forwardB = 0b10;
  }
  else
  { // ID/EX의 rt 값를 그대로 쓰는 경우(forwarding 안하는 경우)
    *forwardB = 0b00;
  }
}

void PipelinedCPU::HazardDetectionUnit(
    const std::bitset<5> *IF_ID_rs, const std::bitset<5> *IF_ID_rt,
    const std::bitset<1> *ID_EX_memRead, const std::bitset<5> *ID_EX_rt,
    std::bitset<1> *PCWrite, std::bitset<1> *IFIDWrite, std::bitset<1> *ctrlSelect)
{
  if (*ID_EX_memRead == 1 && (*ID_EX_rt == *IF_ID_rs || *ID_EX_rt == *IF_ID_rt))
  {
    *PCWrite = 0;
    *IFIDWrite = 0;
    *ctrlSelect = 1;
  }
  else
  {
    *PCWrite = 1;
    *IFIDWrite = 1;
    *ctrlSelect = 0;
  }
}
