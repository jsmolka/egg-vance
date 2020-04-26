#include "sram.h"

SRAM::SRAM(const fs::path& file)
    : Backup(file, Backup::Type::SRAM)
{
    data.resize(0x8000, 0);
}

u8 SRAM::readByte(u32 addr)
{
    return data[addr];
}

void SRAM::writeByte(u32 addr, u8 byte)
{
    data[addr] = byte;
}
