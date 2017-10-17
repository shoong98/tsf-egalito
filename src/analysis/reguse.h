#ifndef EGALITO_ANALYSIS_REGUSE_H
#define EGALITO_ANALYSIS_REGUSE_H

#include <vector>
#include <bitset>

#include "instr/register.h"

class Function;
class Instruction;

class AARCH64RegisterUsageX {
private:
    Function *function;
    PhysicalRegister<AARCH64GPRegister> regX;

    std::vector<Instruction *> xList;

public:
    AARCH64RegisterUsageX(Function *function, AARCH64GPRegister::ID id);

    std::vector<Instruction *> getInstructionList() const { return xList; }
    std::vector<bool> getUnusableRegister();
};

class AARCH64RegisterUsage {
public:
    AARCH64RegisterUsage() {}

    std::vector<int> getAllUseCounts(Function *function);
    std::bitset<32> getUnusedRegisters(Function *function);
};

#endif
