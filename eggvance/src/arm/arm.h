#pragma once

#include "bios.h"
#include "io.h"
#include "pipeline.h"
#include "registers.h"
#include "scheduler/event.h"

class Arm : public Registers
{
public:
    friend class Dma;
    friend class DmaChannel;

    Arm();

    void init();
    void run(int cycles);
    void raise(uint irq);

    u8  readByte(u32 addr, Access access = Access::NonSequential);
    u16 readHalf(u32 addr, Access access = Access::NonSequential);
    u32 readWord(u32 addr, Access access = Access::NonSequential);

    void writeByte(u32 addr, u8  byte, Access access = Access::NonSequential);
    void writeHalf(u32 addr, u16 half, Access access = Access::NonSequential);
    void writeWord(u32 addr, u32 word, Access access = Access::NonSequential);

    uint state = 0;

private:
    using Instruction32 = void(Arm::*)(u32);
    using Instruction16 = void(Arm::*)(u16);

    enum Shift
    {
        kShiftLsl,
        kShiftLsr,
        kShiftAsr,
        kShiftRor
    };

    static const std::array<Instruction32, 4096> instr_arm;
    static const std::array<Instruction16, 1024> instr_thumb;

    template<uint Hash> static constexpr Instruction32 Arm_Decode();
    template<uint Hash> static constexpr Instruction16 Thumb_Decode();

    template<bool Immediate> SHELL_INLINE u32 lsl(u32 value, u32 amount, bool flags = true);
    template<bool Immediate> SHELL_INLINE u32 lsr(u32 value, u32 amount, bool flags = true);
    template<bool Immediate> SHELL_INLINE u32 asr(u32 value, u32 amount, bool flags = true);
    template<bool Immediate> SHELL_INLINE u32 ror(u32 value, u32 amount, bool flags = true);

    SHELL_INLINE u32 log(u32 op1,          bool flags = true);
    SHELL_INLINE u32 add(u32 op1, u32 op2, bool flags = true);
    SHELL_INLINE u32 sub(u32 op1, u32 op2, bool flags = true);
    SHELL_INLINE u32 adc(u32 op1, u32 op2, bool flags = true);
    SHELL_INLINE u32 sbc(u32 op1, u32 op2, bool flags = true);

    u8 readIo(u32 addr);
    void writeIo(u32 addr, u8 byte);

    u32 readUnused() const;
    u32 readHalfRotate(u32 addr, Access access = Access::NonSequential);
    u32 readWordRotate(u32 addr, Access access = Access::NonSequential);
    u32 readByteSignEx(u32 addr, Access access = Access::NonSequential);
    u32 readHalfSignEx(u32 addr, Access access = Access::NonSequential);

    template<uint State> 
    void dispatch();
    void flushHalf();
    void flushWord();

    SHELL_INLINE void tick(int cycles);
    SHELL_INLINE void idle(int cycles = 1);
    SHELL_INLINE void tickRam(int cycles);
    SHELL_INLINE void tickRom(u32 addr, int cycles);
    template<bool Signed>
    SHELL_INLINE void tickMultiply(u32 multiplier);

    void interrupt(u32 pc, u32 lr, Psr::Mode mode);
    void interruptHw();
    void interruptSw();
    void interruptProcess();

    template<u32 Instr> void Arm_BranchExchange(u32 instr);
    template<u32 Instr> void Arm_BranchLink(u32 instr);
    template<u32 Instr> void Arm_DataProcessing(u32 instr);
    template<u32 Instr> void Arm_StatusTransfer(u32 instr);
    template<u32 Instr> void Arm_Multiply(u32 instr);
    template<u32 Instr> void Arm_MultiplyLong(u32 instr);
    template<u32 Instr> void Arm_SingleDataTransfer(u32 instr);
    template<u32 Instr> void Arm_HalfSignedDataTransfer(u32 instr);
    template<u32 Instr> void Arm_BlockDataTransfer(u32 instr);
    template<u32 Instr> void Arm_SingleDataSwap(u32 instr);
    template<u32 Instr> void Arm_SoftwareInterrupt(u32 instr);
    template<u32 Instr> void Arm_CoprocessorDataOperations(u32 instr);
    template<u32 Instr> void Arm_CoprocessorDataTransfers(u32 instr);
    template<u32 Instr> void Arm_CoprocessorRegisterTransfers(u32 instr);
    template<u32 Instr> void Arm_Undefined(u32 instr);

    template<u16 Instr> void Thumb_MoveShiftedRegister(u16 instr);
    template<u16 Instr> void Thumb_AddSubtract(u16 instr);
    template<u16 Instr> void Thumb_ImmediateOperations(u16 instr);
    template<u16 Instr> void Thumb_AluOperations(u16 instr);
    template<u16 Instr> void Thumb_HighRegisterOperations(u16 instr);
    template<u16 Instr> void Thumb_LoadPcRelative(u16 instr);
    template<u16 Instr> void Thumb_LoadStoreRegisterOffset(u16 instr);
    template<u16 Instr> void Thumb_LoadStoreByteHalf(u16 instr);
    template<u16 Instr> void Thumb_LoadStoreImmediateOffset(u16 instr);
    template<u16 Instr> void Thumb_LoadStoreHalf(u16 instr);
    template<u16 Instr> void Thumb_LoadStoreSpRelative(u16 instr);
    template<u16 Instr> void Thumb_LoadRelativeAddress(u16 instr);
    template<u16 Instr> void Thumb_AddOffsetSp(u16 instr);
    template<u16 Instr> void Thumb_PushPopRegisters(u16 instr);
    template<u16 Instr> void Thumb_LoadStoreMultiple(u16 instr);
    template<u16 Instr> void Thumb_ConditionalBranch(u16 instr);
    template<u16 Instr> void Thumb_SoftwareInterrupt(u16 instr);
    template<u16 Instr> void Thumb_UnconditionalBranch(u16 instr);
    template<u16 Instr> void Thumb_LongBranchLink(u16 instr);
    template<u16 Instr> void Thumb_Undefined(u16 instr);

    Pipeline pipe;
    int cycles = 0;

    struct Events
    {
        Event interrupt;
    } events;

    struct Irq
    {
        bool servable() const;
        bool interrupted() const;

        IrqMaster  master;
        IrqEnable  enable;
        IrqRequest request;
    } irq;

    struct Prefetch
    {
        int active = 0;
        int cycles = 0;
    } prefetch;

    WaitControl  waitcnt;
    HaltControl  haltcnt;
    Register<u8> postflg;

    Bios bios;
    Ram<0x40000> ewram{};
    Ram<0x08000> iwram{};
};

inline Arm arm;

#include "arithmetic.inl"
#include "shifts.inl"
#include "ticks.inl"
