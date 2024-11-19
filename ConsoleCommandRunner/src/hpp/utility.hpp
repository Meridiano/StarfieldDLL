#pragma once

static bool saveLoaded = false;

namespace CCRUtility {

    void ConsoleExecute(std::string command) {
        static REL::Relocation<void(**)> BGSScaleFormManager{ REL::ID(879512) };
        static REL::Relocation<void(*)(void*, const char*)> ExecuteCommand{ REL::ID(166307) };
        ExecuteCommand(*BGSScaleFormManager, command.data());
    }

    RE::TESObjectCELL* GetParentCell(std::int64_t a1, std::int64_t a2, RE::TESObjectREFR* a3) {
        using func_t = decltype(&GetParentCell);
        REL::Relocation<func_t> func{ REL::ID(172297) };
        return func(a1, a2, a3);
    }

    std::uint32_t PlayerCellFormID() {
        if (auto pc = RE::PlayerCharacter::GetSingleton(); pc)
            if (auto cell = GetParentCell(NULL, NULL, pc); cell)
                return (cell->formID);
        return 0;
    }

    int GameLoadType() {
        if (saveLoaded) return 2;
        if (PlayerCellFormID() > 0) {
            auto gameDaysPassed = RE::TESForm::LookupByID<RE::TESGlobal>(0x39);
            if (gameDaysPassed && gameDaysPassed->GetValue() < 0.334F) return 1;
        }
        return -1;
    }

}