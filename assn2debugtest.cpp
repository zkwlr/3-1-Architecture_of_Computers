#include <bitset>
#include <cassert>
#include <cstdio>
#include <cstdlib>

#define MEMORY_SIZE (32 * 1024 * 1024) // <-- 32-MB memory

class Memory
{
public:
    enum Endianness
    {
        LittleEndian,
        BigEndian
    };
    Memory(
        const Endianness endianness,
        const char *initFileName = nullptr) : m_endianness(endianness), m_memory(new std::bitset<8>[MEMORY_SIZE])
    {
        if (initFileName != nullptr)
        {
            // Each line of the memory initialization file consists of:
            //   1) the starting memory address of a 32-bit data in hexadecimal value
            //   2) the eight-digit hexadecimal value of the data
            // For example, "1000 ABCD1234" stores 0xABCD1234 to memory addresses from 0x1000 to 0x1003.
            FILE *initFile = fopen(initFileName, "r");
            assert(initFile != NULL);
            std::uint32_t addr, value;
            while (fscanf(initFile, " %x %x", &addr, &value) == 2)
            {
                printf("INFO: memory[0x%08lx..0x%08lx] <-- 0x%08lx\n",
                       (unsigned long)addr, (unsigned long)(addr + 3),
                       (unsigned long)value);
                if (m_endianness == LittleEndian)
                {
                    m_memory[addr + 0] = (std::uint8_t)(value % 0x100);
                    value >>= 8;
                    m_memory[addr + 1] = (std::uint8_t)(value % 0x100);
                    value >>= 8;
                    m_memory[addr + 2] = (std::uint8_t)(value % 0x100);
                    value >>= 8;
                    m_memory[addr + 3] = (std::uint8_t)(value % 0x100);
                }
                else
                { // m_endianness == BigEndian
                    m_memory[addr + 3] = (std::uint8_t)(value % 0x100);
                    value >>= 8;
                    m_memory[addr + 2] = (std::uint8_t)(value % 0x100);
                    value >>= 8;
                    m_memory[addr + 1] = (std::uint8_t)(value % 0x100);
                    value >>= 8;
                    m_memory[addr + 0] = (std::uint8_t)(value % 0x100);
                }
            }
            fclose(initFile);
        }
    }
    void printMemory()
    {
        for (size_t i = 0; i < MEMORY_SIZE; i += 4)
        {
            if (m_memory[i].any() || m_memory[i + 1].any() || m_memory[i + 2].any() || m_memory[i + 3].any())
            {
                std::uint32_t value = 0;
                if (m_endianness == LittleEndian)
                {
                    value += (m_memory[i + 3].to_ulong());
                    value <<= 8;
                    value += (m_memory[i + 2].to_ulong());
                    value <<= 8;
                    value += (m_memory[i + 1].to_ulong());
                    value <<= 8;
                    value += (m_memory[i + 0].to_ulong());
                }
                else
                { // m_endianness == BigEndian
                    value += (m_memory[i + 0].to_ulong());
                    value <<= 8;
                    value += (m_memory[i + 1].to_ulong());
                    value <<= 8;
                    value += (m_memory[i + 2].to_ulong());
                    value <<= 8;
                    value += (m_memory[i + 3].to_ulong());
                }
                printf("  memory[0x%08lx..0x%08lx] = 0x%08lx\n", i, i + 3,
                       (unsigned long)value);
            }
        }
    }
    void access(
        const std::bitset<32> *address, const std::bitset<32> *writeData,
        const std::bitset<1> *memRead, const std::bitset<1> *memWrite,
        std::bitset<32> *readData);
    ~Memory()
    {
        delete[] m_memory;
    }

private:
    // memory
    Endianness m_endianness;
    std::bitset<8> *m_memory;
};

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
        {                                                                              //*writeData의 Least Significant Byte부터 넣는다.(8bit씩)
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

class RegisterFile
{
public:
    RegisterFile(const char *initFileName = nullptr)
    {
        for (size_t i = 0; i < 32; i++)
        {
            m_registers[i].reset();
        }
        if (initFileName != nullptr)
        {
            // Each line of the memory initialization file consists of:
            //   1) the target register index in decimal value
            //   2) the eight-digit hexadecimal value of the data
            // For example, "10 ABCD1234" stores 0xABCD1234 to register #10.
            FILE *initFile = fopen(initFileName, "r");
            assert(initFile != NULL);
            std::uint32_t reg, val;
            while (fscanf(initFile, " %u %x", &reg, &val) == 2)
            {
                if (reg != 0)
                {
                    printf("INFO: $%02u <-- 0x%08lx\n", reg, (unsigned long)val);
                    m_registers[reg] = val;
                }
            }
        }
    }
    void printRegisters()
    {
        for (size_t i = 0; i < 32; i++)
        {
            printf("  $%02lu = 0x%08lx\n", i, m_registers[i].to_ulong());
        }
    }
    void access(
        const std::bitset<5> *readRegister1, const std::bitset<5> *readRegister2,
        const std::bitset<5> *writeRegister, const std::bitset<32> *writeData,
        const std::bitset<1> *regWrite,
        std::bitset<32> *readData1, std::bitset<32> *readData2);

private:
    // registers
    std::bitset<32> m_registers[32];
};

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
        //  그러므로 reg에서 read data를 할 때에 writedata가 작동해서 writeReg에 쓰레기값이 들어가도 된다.
        //  어쩌피 마지막 WB 과정에서 writeReg에 원하는 값이 기록된 writeBackData를 저장하기 때문이다.
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
    {                      // If select = 0
        *output = *input0; // output = input0
    }
    else
    {                      // If select = 1
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

    // wire(datapath), port 배치
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

    // Fetch
    *memRead = 0b1;                                                        // instruction memory에서 읽어야 하므로 memRead 신호는 1이다.
    m_instMemory->access(&m_PC, nullptr, memRead, memWrite, readinstData); // readData에 현재 PC가 가리키는 instruction 저장
    m_instruction = *readinstData;                                         // readData port에서 wire로 instruction code를 보냄
    // m_PC = m_PC.to_ulong() + 4; //PC = PC + 4
    std::bitset<32> four(4);
    Add<32>(&m_PC, &four, &m_PC); // PC = PC + 4

    // Decode - Parse the fetched instruction
    std::bitset<6> opcode = m_instruction.to_ulong() >> 26;         // instruction code의 상위 6bit만 남게 left shift
    std::bitset<5> rs = (m_instruction.to_ulong() >> 21) & 0b11111; // 0b11111 = and 연산통해 하위 5bit의 값만 가져옴
    std::bitset<5> rt = (m_instruction.to_ulong() >> 16) & 0b11111;
    std::bitset<5> rd = (m_instruction.to_ulong() >> 11) & 0b11111;
    std::bitset<6> funct = m_instruction.to_ulong() & 0b111111;
    std::bitset<16> immediate = m_instruction.to_ulong() & 0xFFFF; // 하위 16bit만 가져옴

    // Decode - Set control signals by opcode
    Control(&opcode, regDst, branch, memRead, memToReg, aluOp, memWrite, aluSrc, regWrite);

    // Register File
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

    // Data Memory
    std::bitset<32> *address = aluResult;            // aluResult port ->(wire)-> address port
    std::bitset<32> *m_writeData = readData2;        // readData2 port ->(wire)-> m_writeData port
    std::bitset<32> *readData = new std::bitset<32>; // output port

    // memRead, memWrite의 상태에 따라 read할지, write할지, 접근 안할건지 결정
    // (Data memory 앞에 있는 MUX에서 WB은 언제나 일어남, regWrite 신호에 따라 WriteRegister에 writeData를 write할지 결정)
    m_dataMemory->access(address, m_writeData, memRead, memWrite, readData);

    // Write Back
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
    // std::bitset<32> input0(0x11111111), input1(0x10101010), output0, output1;
    // std::bitset<1> selector0(0), selector1(1);
    // cpu->Mux<32>(&input0, &input1, &selector0, &output0);
    // cpu->Mux<32>(&input0, &input1, &selector1, &output1);
    // assert(output0.to_ulong() == 0x11111111 && output1.to_ulong() == 0x10101010);

    cpu->printPVS();
    for (size_t i = 0; i < numCycles; i++)
    {
        cpu->advanceCycle();
        cpu->printPVS();
    }

    delete cpu;

    return 0;
}
