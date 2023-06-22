#include "PipelinedCPU.hpp"

void PipelinedCPU::InstructionFetch() {
  /*****************************/
  /*********** FIXME ***********/
  /*****************************/
}

void PipelinedCPU::InstructionDecode() {
  /*****************************/
  /*********** FIXME ***********/
  /*****************************/
}

void PipelinedCPU::Execute() {
  /*****************************/
  /*********** FIXME ***********/
  /*****************************/
}

void PipelinedCPU::MemoryAccess() {
  /*****************************/
  /*********** FIXME ***********/
  /*****************************/
}

void PipelinedCPU::WriteBack() {
  /*****************************/
  /*********** FIXME ***********/
  /*****************************/
}

void PipelinedCPU::ForwardingUnit(
  const std::bitset<5> *ID_EX_rs, const std::bitset<5> *ID_EX_rt,
  const std::bitset<1> *EX_MEM_regWrite, const std::bitset<5> *EX_MEM_rd,
  const std::bitset<1> *MEM_WB_regWrite, const std::bitset<5> *MEM_WB_rd,
  std::bitset<2> *forwardA, std::bitset<2> *forwardB
) {
  /*****************************/
  /*********** FIXME ***********/
  /*****************************/
}

void PipelinedCPU::HazardDetectionUnit(
  const std::bitset<5> *IF_ID_rs, const std::bitset<5> *IF_ID_rt,
  const std::bitset<1> *ID_EX_memRead, const std::bitset<5> *ID_EX_rt,
  std::bitset<1> *PCWrite, std::bitset<1> *IFIDWrite, std::bitset<1> *ctrlSelect
) {
  /*****************************/
  /*********** FIXME ***********/
  /*****************************/
}

