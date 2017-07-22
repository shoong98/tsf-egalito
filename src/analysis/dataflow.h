#ifndef EGALITO_ANALYSIS_DATAFLOW_H
#define EGALITO_ANALYSIS_DATAFLOW_H

#include "analysis/usedef.h"
#include "analysis/liveregister.h"

#ifdef ARCH_AARCH64
class Module;

class DataFlow {
private:
    std::map<Function *, UseDef *> flowList;
    std::vector<UDRegMemWorkingSet *> workingList;
    std::vector<UDConfiguration *> configList;
    std::vector<ControlFlowGraph *> graphList;

public:
    ~DataFlow();
    void addUseDefFor(Function *function);
    void adjustCallUse(LiveRegister *live, Function *function, Module *module);
    UDRegMemWorkingSet *getWorkingSet(Function *function);

private:
    bool isTLSdescResolveCall(UDState *state, Module *module);
};
#endif

#endif
