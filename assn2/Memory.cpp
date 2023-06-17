#include "Memory.hpp"

void Memory::access(
    const std::bitset<32> *address, const std::bitset<32> *writeData,
    const std::bitset<1> *memRead, const std::bitset<1> *memWrite,
    std::bitset<32> *readData)
{
  assert(memRead != nullptr);
  assert(memWrite != nullptr);
  if (memRead->all() && memWrite->all())
  {
    printf("ERROR: Both `memRead' and `memWrite' are set.\n");
    assert(!(memRead->all() && memWrite->all()));
  }
  else if (memRead->all())
  {
    assert(address != nullptr);
    assert(readData != nullptr);
    // read memory
    readData->reset(); // readData를 0으로 초기화
    if (m_endianness == LittleEndian)
    {
      *readData = readData->to_ulong() + m_memory[address->to_ulong() + 3].to_ulong();
      *readData <<= 8;
      *readData = readData->to_ulong() + m_memory[address->to_ulong() + 2].to_ulong();
      *readData <<= 8;
      *readData = readData->to_ulong() + m_memory[address->to_ulong() + 1].to_ulong();
      *readData <<= 8;
      *readData = readData->to_ulong() + m_memory[address->to_ulong() + 0].to_ulong();
    }
    else // m_endianness == BigEndian
    {
      *readData = readData->to_ulong() + m_memory[address->to_ulong() + 0].to_ulong();
      *readData <<= 8;
      *readData = readData->to_ulong() + m_memory[address->to_ulong() + 1].to_ulong();
      *readData <<= 8;
      *readData = readData->to_ulong() + m_memory[address->to_ulong() + 2].to_ulong();
      *readData <<= 8;
      *readData = readData->to_ulong() + m_memory[address->to_ulong() + 3].to_ulong();
    }
  }
  else if (memWrite->all())
  {
    assert(address != nullptr);
    assert(writeData != nullptr);
    // write memory
    if (m_endianness == LittleEndian)
    {                                                                            //*writeData의 Least Significant Byte부터 넣는다.(8bit씩)
      m_memory[address->to_ulong() + 0] = writeData->to_ulong() % 0x100;         // 나머지 연산으로 맨 뒤 1byte 저장
      m_memory[address->to_ulong() + 1] = (writeData->to_ulong() >> 8) % 0x100;  // 1byte shift 후 2번째 LSB 저장
      m_memory[address->to_ulong() + 2] = (writeData->to_ulong() >> 16) % 0x100; // 2byte shift 후 3번째 LSB 저장
      m_memory[address->to_ulong() + 3] = (writeData->to_ulong() >> 24) % 0x100; // 3byte shift 후 MSB 저장
    }
    else // m_endianness == BigEndian
    {    //*writeData의 Most Significant Byte부터 넣는다.(8bit씩)
      m_memory[address->to_ulong() + 3] = writeData->to_ulong() % 0x100;
      m_memory[address->to_ulong() + 2] = (writeData->to_ulong() >> 8) % 0x100;
      m_memory[address->to_ulong() + 1] = (writeData->to_ulong() >> 16) % 0x100;
      m_memory[address->to_ulong() + 0] = (writeData->to_ulong() >> 24) % 0x100;
    }
  }
}
