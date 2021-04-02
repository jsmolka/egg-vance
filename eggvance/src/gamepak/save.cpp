#include "save.h"

#include <algorithm>
#include <string_view>
#include <utility>
#include <shell/errors.h>

#include "base/sdl2.h"

Save::Save()
    : type(Type::None)
{

}

Save::Save(Type type)
    : type(type)
{

}

Save::~Save()
{
    if (changed
        && !file.empty()
        && !data.empty()
        && type != Type::None)
    {
        if (fs::write(file, data) != fs::Status::Ok)
            showMessageBox("Warning", "Cannot write save: {}", file);
    }
}

Save::Type Save::parse(const Rom& rom)
{
    static constexpr std::tuple<std::string_view, Save::Type> kSignatures[6] =
    {
        { "SRAM_V",     Save::Type::Sram      },
        { "SRAM_F_V",   Save::Type::Sram      },
        { "EEPROM_V",   Save::Type::Eeprom    },
        { "FLASH_V",    Save::Type::Flash512  },
        { "FLASH512_V", Save::Type::Flash512  },
        { "FLASH1M_V",  Save::Type::Flash1024 }
    };

    for (std::size_t addr = Rom::kSizeHeader; addr < rom.size(); addr += 4)
    {
        for (const auto& [signature, type] : kSignatures)
        {
            if (addr + signature.size() < rom.size()
                    && std::equal(signature.begin(), signature.end(), rom.begin() + addr))
                return type;
        }
    }
    return Type::None;
}

void Save::init(const fs::path& file)
{
    if (fs::is_regular_file(file))
    {
        if (fs::read(file, data) != fs::Status::Ok)
            throw shell::Error("Cannot read save: {}", file);

        if (!valid(data.size()))
            throw shell::Error("Bad save size: {}", data.size());
    }
    this->file = file;
}

void Save::reset()
{

}

u8 Save::read(u32 addr)
{
    return 0;
}

void Save::write(u32 addr, u8 byte)
{

}

bool Save::valid(uint size) const
{
    return size == 0;
}
