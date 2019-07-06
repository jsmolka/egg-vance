#pragma once

#include "common/integer.h"
#include "mmu/mmu.h"
#include "decoder.h"
#include "registers.h"

class ARM
{
public:
    ARM(MMU& mmu);

    void reset();

    void interrupt();

    int step();

private:
    struct PipeState
    {
        u32 data;
        bool refill;
        ArmInstr arm;
        ThumbInstr thumb;
    };

    MMU& mmu;
    Registers regs;

    void fetch(PipeState &state);
    void decode(PipeState &state);
    void execute(PipeState &state);
    void advance();

    void debug(PipeState& state);

    PipeState pipe[3];
    bool needs_flush;
    void flush();

    void updateZ(u32 result);
    void updateN(u32 result);
    void updateC(u32 op1, u32 op2, bool addition);
    void updateV(u32 op1, u32 op2, bool addition);

    void logical(u32 result);
    void logical(u32 result, bool carry);
    void arithmetic(u32 op1, u32 op2, bool addition);

    u32 lsl(u32 value, int offset, bool& carry);
    u32 lsr(u32 value, int offset, bool& carry, bool immediate = true);
    u32 asr(u32 value, int offset, bool& carry, bool immediate = true);
    u32 ror(u32 value, int offset, bool& carry, bool immediate = true);

    u32 ldr(u32 addr);
    u32 ldrh(u32 addr);
    u32 ldrsh(u32 addr);

    void moveShiftedRegister(u16 instr);
    void addSubImmediate(u16 instr);
    void addSubMovCmpImmediate(u16 instr);
    void aluOperations(u16 instr);
    void highRegisterBranchExchange(u16 instr);
    void loadPcRelative(u16 instr);
    void loadStoreRegisterOffset(u16 instr);
    void loadStoreHalfSigned(u16 instr);
    void loadStoreImmediateOffset(u16 instr);
    void loadStoreHalf(u16 instr);
    void loadStoreSpRelative(u16 instr);
    void loadAddress(u16 instr);
    void addOffsetSp(u16 instr);
    void pushPopRegisters(u16 instr);
    void loadStoreMultiple(u16 instr);
    void conditionalBranch(u16 instr);
    void swiThumb(u16 instr);
    void unconditionalBranch(u16 instr);
    void longBranchLink(u16 instr);

    u32 shiftedRegister(int data, bool& carry);
    u32 rotatedImmediate(int data, bool& carry);

    void branchExchange(u32 instr);
    void branchLink(u32 instr);
    void dataProcessing(u32 instr);
    void psrTransfer(u32 instr);
    void multiply(u32 instr);
    void multiplyLong(u32 instr);
    void singleTransfer(u32 instr);
    void halfSignedTransfer(u32 instr);
    void blockTransfer(u32 instr);
    void singleSwap(u32 instr);
    void swiArm(u32 instr);

    enum MemoryAccess
    {
        NSEQ,
        SEQ
    };

    int cycles;

    void cycle();
    void cycle(u32 addr, MemoryAccess access);
    void cycleMultiplication(u32 multiplier, bool allow_ones);
};
