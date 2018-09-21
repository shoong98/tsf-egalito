#ifndef EGALITO_GENERATE_CONCRETE_DEFERRED_H
#define EGALITO_GENERATE_CONCRETE_DEFERRED_H

#include <vector>
#include "deferred.h"
#include "chunk/link.h"
#include "elf/elfxx.h"

class Section;
class SectionRef;
class Symbol;
class Function;
class Instruction;
class ElfSpace;
class Chunk;
class SectionList;

class SymbolInTable {
public:
    enum type_t {
        TYPE_NULL,
        TYPE_SECTION,
        TYPE_LOCAL,
        TYPE_UNDEF,
        TYPE_GLOBAL
    };
private:
    type_t type;
    Symbol *sym;
public:
    SymbolInTable(type_t type = TYPE_NULL, Symbol *sym = nullptr)
        : type(type), sym(sym) {}
    bool operator < (const SymbolInTable &other) const;
    bool operator == (const SymbolInTable &other) const;
    Symbol *get() const { return sym; }
    std::string getName() const;
};

/** Symbol table, either .strtab or .dynstr. The ordering of symbols
    is determined by the symbolCompare function.
*/
class SymbolTableContent : public DeferredMap<SymbolInTable, ElfXX_Sym> {
public:
    typedef DeferredValueImpl<ElfXX_Sym> DeferredType;
private:
    DeferredStringList *strtab;
    std::vector<DeferredType *> sectionSymbols;
    int firstGlobalIndex;
public:
    SymbolTableContent(DeferredStringList *strtab)
        : strtab(strtab), firstGlobalIndex(0) {}

    void addNullSymbol();
    void addSectionSymbol(Symbol *sym);
    DeferredType *addSymbol(Function *func, Symbol *sym);
    DeferredType *addUndefinedSymbol(Symbol *sym);

    size_t indexOfSectionSymbol(const std::string &section,
        SectionList *sectionList);
    int getFirstGlobalIndex() const { return firstGlobalIndex; }
public:
    static SymbolInTable::type_t getTypeFor(Function *func);
};

class ShdrTableContent : public DeferredMap<Section *, ElfXX_Shdr> {
public:
    typedef DeferredValueImpl<ElfXX_Shdr> DeferredType;
public:
    DeferredType *add(Section *section);
};

class SegmentInfo {
private:
    ElfXX_Word type;
    ElfXX_Word flags;
    address_t alignment;
    size_t additionalMemSize;
    std::vector<Section *> containsList;
public:
    SegmentInfo(ElfXX_Word type, ElfXX_Word flags, address_t alignment)
        : type(type), flags(flags), alignment(alignment), additionalMemSize(0) {}

    void setAdditionalMemSize(size_t a) { additionalMemSize = a; }
    void addContains(Section *section) { containsList.push_back(section); }

    ElfXX_Word getType() const { return type; }
    ElfXX_Word getFlags() const { return flags; }
    address_t getAlignment() const { return alignment; }
    size_t getAdditionalMemSize() const { return additionalMemSize; }
    std::vector<Section *> &getContainsList() { return containsList; }
};

class PhdrTableContent : public DeferredMap<SegmentInfo *, ElfXX_Phdr> {
public:
    typedef DeferredValueImpl<ElfXX_Phdr> DeferredType;
private:
    SectionList *sectionList;
public:
    PhdrTableContent(SectionList *sectionList) : sectionList(sectionList) {}

    DeferredType *add(SegmentInfo *segment);
    DeferredType *add(SegmentInfo *segment, address_t address);
    void assignAddressesToSections(SegmentInfo *segment, address_t addr);
};

class PagePaddingContent : public DeferredValue {
private:
    static const address_t PAGE_SIZE = 0x200000;
private:
    Section *previousSection;
    address_t desiredOffset;
public:
    PagePaddingContent(Section *previousSection, address_t desiredOffset = 0)
        : previousSection(previousSection), desiredOffset(desiredOffset) {}

    virtual size_t getSize() const;
    virtual void writeTo(std::ostream &stream);
};

class RelocSectionContent : public DeferredMap<address_t, ElfXX_Rela> {
public:
    typedef DeferredValueImpl<ElfXX_Rela> DeferredType;
private:
    SectionRef *outer;
    SectionList *sectionList;
    ElfSpace *elfSpace;
public:
    RelocSectionContent(SectionRef *outer, SectionList *sectionList,
        ElfSpace *elfSpace) : outer(outer), sectionList(sectionList),
        elfSpace(elfSpace) {}

    Section *getTargetSection();

    DeferredType *add(Chunk *source, Link *link);
private:
    DeferredType *makeDeferredForLink(Instruction *source);
    DeferredType *addConcrete(Instruction *source, DataOffsetLink *link);
    DeferredType *addConcrete(Instruction *source, PLTLink *link);
    DeferredType *addConcrete(Instruction *source, SymbolOnlyLink *link);
};

class DataSection;
class RelocSectionContent2 : public DeferredMap<address_t, ElfXX_Rela> {
public:
    typedef DeferredValueImpl<ElfXX_Rela> DeferredType;
private:
    SectionList *sectionList;
    SectionRef *other;
public:
    RelocSectionContent2(SectionList *sectionList, SectionRef *other)
        : sectionList(sectionList), other(other) {}

    Section *getTargetSection();

    DeferredType *addDataRef(address_t source, address_t target,
        DataSection *targetSection);
};

class DataVariable;
class DataRelocSectionContent : public DeferredMap<address_t, ElfXX_Rela> {
public:
    typedef DeferredValueImpl<ElfXX_Rela> DeferredType;
private:
    SectionRef *outer;
    SectionList *sectionList;
public:
    DataRelocSectionContent(SectionRef *outer, SectionList *sectionList)
        : outer(outer), sectionList(sectionList) {}

    Section *getTargetSection();

    DeferredType *addUndefinedRef(DataVariable *var, LDSOLoaderLink *link);
    DeferredType *addDataRef(address_t source, address_t target,
        DataSection *targetSection);
    DeferredType *addTLSOffsetRef(address_t source, TLSDataOffsetLink *link);
};

class DynamicDataPair {
private:
    unsigned long key;
    unsigned long value;
public:
    DynamicDataPair(unsigned long key, unsigned long value = 0)
        : key(key), value(value) {}
    unsigned long getKey() const { return key; }
    unsigned long getValue() const { return value; }
    void setKey(unsigned long key) { this->key = key; }
    void setValue(unsigned long value) { this->value = value; }
};

class DynamicSectionContent : public DeferredList<DynamicDataPair> {
public:
    typedef DeferredValueImpl<DynamicDataPair> DeferredType;

    DeferredType *addPair(unsigned long key,
        std::function<address_t ()> generator);
    DeferredType *addPair(unsigned long key, unsigned long value);
};

class InitArraySectionContent : public DeferredValue {
private:
    std::vector<std::function<address_t ()>> array;
    std::vector<std::function<void ()>> callbacks;
public:
    void addPointer(std::function<address_t ()> func) { array.push_back(func); }
    void addCallback(std::function<void ()> func) { callbacks.push_back(func); }
    virtual size_t getSize() const { return array.size() * sizeof(address_t); }
    virtual void writeTo(std::ostream &stream);
};

#endif
