#include "RegisterFile.hpp"

void RegisterFile::access(
    const std::bitset<5> *readRegister1, const std::bitset<5> *readRegister2,
    const std::bitset<5> *writeRegister, const std::bitset<32> *writeData,
    const std::bitset<1> *regWrite,
    std::bitset<32> *readData1, std::bitset<32> *readData2)
{
  if (regWrite->all())
  { // regWrite = 1
    assert(writeRegister != nullptr && writeData != nullptr);
    // Read&Write register
    //  writeRegister가 const여도 되는이유
    //  : bitset<5> 자료형 writeRegister는 이게 무슨 register인지(0,1,...,31)를 기록하는 것 뿐이고
    //    실제로 register에 값을 쓰는 코드는 m_registers 배열에서 이루어지기 때문에
    //    writeRegister 값은 access 함수에서 변하는게 아니라
    //    decode 단계에서 writereg에 rt나 rd를 받으면서 바뀐다.
    // regWrite = 1이어도 readRegister에서 값은 불러와야 한다.
    *readData1 = m_registers[readRegister1->to_ulong()];
    *readData2 = m_registers[readRegister2->to_ulong()];
    //  regWrite = 1이면 값을 저장할 레지스터에 WB할 Data를 저장한다.
    //  어떤 rt나 rd 레지스터를 writeReg로 지정한다면, 그 reg에 원래 있던 값은 더이상 생각하지 않아도 된다.
    //  readData1, readData2에 기록될 reg가 아니기 때문이다.
    //  그러므로 reg에서 read data를 할 때에 writedata가 작동해서 writeRegister에 쓰레기값이 들어가도 된다.
    //  어쩌피 마지막 WB 과정에서 writeReg에 원하는 값이 기록된 writeBackData를 저장하기 때문이다.
    //  (writeRegister나 writeData가 nullptr거나 가리키는 주소가 지정되지 않은 경우만 아니면 됨, 그렇게 되면 segmentatioin fault 오류 발생 - 메모리 주소 참조 과정에서의 오류)
    //  이러한 segmentation fault 방지를 위해 모든 wire와 port를 new로 동적 할당하고 사용 후 해제하였다.
    m_registers[writeRegister->to_ulong()] = *writeData;
  }
  else
  { // regWrite = 0
    assert(readData1 != nullptr && readRegister1 != nullptr && readData2 != nullptr && readRegister2 != nullptr);
    // only Read register
    //  Register file에 저장되어있는 값을 토대로 regWrite = 0이면
    //  readRegister1, readRegister2에 해당하는 값을 readData1, readData2에 저장한다.
    //  writeRegister에는 저장하지 않는다.
    *readData1 = m_registers[readRegister1->to_ulong()];
    *readData2 = m_registers[readRegister2->to_ulong()];
  }
}
