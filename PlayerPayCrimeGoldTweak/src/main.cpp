namespace PPCGUtility {

    void ConsoleExecute(std::string command) {
        using typeM = void*;
        using typeF = void(typeM, const char*);
        static REL::Relocation<typeM*> manager{ REL::ID(938528) };
        static REL::Relocation<typeF*> function{ REL::ID(113576) };
        function(*manager, command.data());
    }

    void SetCrimeGold(RE::Actor* actor, RE::TESFaction* faction, bool violent, std::int32_t value) {
        static REL::Relocation<decltype(&SetCrimeGold)> SetCrimeGoldNative{ REL::ID(102824) };
        return SetCrimeGoldNative(actor, faction, violent, value);
    }

    std::vector<std::string> Split(std::string input, char delim) {
        std::vector<std::string> result;
        std::stringstream stream(input);
        std::string item;
        while (std::getline(stream, item, delim)) result.push_back(item);
        return result;
    }

    template <typename T>
    T* GetMember(const void* base, std::ptrdiff_t offset) {
        auto address = std::uintptr_t(base) + offset;
        REL::Relocation<T*> reloc{ address };
        return reloc.get();
    }

    std::string GetPluginName(RE::TESForm* arg) {
        auto tesDH = RE::TESDataHandler::GetSingleton();
        if (!tesDH) REX::FAIL("TESDataHandler not found");
        // lists
        using FL = RE::BSTArray<RE::TESFile*>;
        auto listFull = GetMember<FL>(tesDH, 0x1580);
        auto listSmall = GetMember<FL>(tesDH, 0x1590);
        auto listMedium = GetMember<FL>(tesDH, 0x15A0);
        // lambdas
        auto GetIndex = [](RE::TESForm* form) {
            std::uint32_t id = form ? form->formID : 0;
            switch (id >> 24) {
                case 0xFE: return std::pair(1, (id >> 12) & 0xFFF);
                case 0xFD: return std::pair(2, (id >> 16) & 0xFF);
            }
            return std::pair(0, id >> 24);
        };
        auto GetName = [](FL* list, std::uint32_t index) {
            std::string result("");
            if (index < list->size())
                if (auto file = list->operator[](index); file)
                    result = file->fileName;
            return result;
        };
        // execution
        auto index = GetIndex(arg);
        switch (index.first) {
            case 0: return GetName(listFull, index.second);
            case 1: return GetName(listSmall, index.second);
            case 2: return GetName(listMedium, index.second);
        }
        return std::string("");
    }

}

namespace PPCGHook {

    std::pair<bool, bool> Process(std::string source, RE::Actor* a_actor, RE::TESFaction* a_faction, bool a_goToJail, bool a_removeStolenItems) {
        REX::INFO("PlayerPayCrimeGold.{} with faction {:08X}", source, a_faction->formID);
        bool goToJail = a_goToJail;
        bool removeStolenItems = a_removeStolenItems;
        bool payCrimeGold = true;
        // load ini
        CSimpleIniA ini;
        ini.SetUnicode();
        SI_Error iniResult = ini.LoadFile("Data\\SFSE\\Plugins\\PlayerPayCrimeGoldTweak.ini");
        if (iniResult < 0) REX::INFO("Settings file error / Input values = {:X} {:X} {:X}", goToJail, removeStolenItems, payCrimeGold);
        else {
            // general
            std::string vanillaPlugins = ini.GetValue("General", "VanillaPlugins", 0);
            // modes
            std::int32_t goToJailMode = ini.GetLongValue("Modes", "GoToJailMode", 0);
            std::int32_t removeStolenItemsMode = ini.GetLongValue("Modes", "RemoveStolenItemsMode", 0);
            std::int32_t payCrimeGoldMode = ini.GetLongValue("Modes", "PayCrimeGoldMode", 0);
            // values
            bool goToJailValue = ini.GetBoolValue("Values", "GoToJailValue", false);
            bool removeStolenItemsValue = ini.GetBoolValue("Values", "RemoveStolenItemsValue", false);
            bool payCrimeGoldValue = ini.GetBoolValue("Values", "PayCrimeGoldValue", false);
            // check faction
            bool factionIsVanilla = [](RE::TESFaction* faction, std::string plugins) {
                auto name = PPCGUtility::GetPluginName(faction);
                if (auto length = name.size(); length) {
                    auto list = PPCGUtility::Split(plugins, '|');
                    for (auto plugin : list)
                        if (plugin.size() == length)
                            if (strnicmp(plugin.data(), name.data(), length) == 0)
                                return true;
                }
                return false;
            }(a_faction, vanillaPlugins);
            // process jail
            if (goToJailMode == 1 && factionIsVanilla) { // replace vanilla
                goToJail = goToJailValue;
            } else if (goToJailMode == 2 && !factionIsVanilla) { // replace non-vanilla
                goToJail = goToJailValue;
            } else if (goToJailMode == 3) { // replace any
                goToJail = goToJailValue;
            }
            // process items
            if (removeStolenItemsMode == 1 && factionIsVanilla) { // replace vanilla
                removeStolenItems = removeStolenItemsValue;
            } else if (removeStolenItemsMode == 2 && !factionIsVanilla) { // replace non-vanilla
                removeStolenItems = removeStolenItemsValue;
            } else if (removeStolenItemsMode == 3) { // replace any
                removeStolenItems = removeStolenItemsValue;
            }
            // process gold
            if (payCrimeGoldMode == 1 && factionIsVanilla) { // replace vanilla
                payCrimeGold = payCrimeGoldValue;
            } else if (payCrimeGoldMode == 2 && !factionIsVanilla) { // replace non-vanilla
                payCrimeGold = payCrimeGoldValue;
            } else if (payCrimeGoldMode == 3) { // replace any
                payCrimeGold = payCrimeGoldValue;
            }
            // show result
            REX::INFO("Settings file parsed / Vanilla faction = {:X} / Result values = {:X} {:X} {:X}", factionIsVanilla, goToJail, removeStolenItems, payCrimeGold);
        }
        // allow landing if actor is in space
        if (a_actor->IsInSpace(true) && !goToJail) {
            std::string command = std::format("CGF {}{}.{}{}", '"', "PlayerPayCrimeGoldTweak", "AllowLanding", '"');
            PPCGUtility::ConsoleExecute(command);
        }
        // keep actor's gold
        if (!payCrimeGold) {
            PPCGUtility::SetCrimeGold(a_actor, a_faction, true, 0);
            PPCGUtility::SetCrimeGold(a_actor, a_faction, false, 0);
        }
        return { goToJail, removeStolenItems };
    }

    class PlayerPayCrimeGoldHook {
    private:
        struct PlayerPayCrimeGold {
            static void NEW_V(RE::Actor* a_actor, RE::TESFaction* a_faction, bool a_goToJail, bool a_removeStolenItems) {
                auto result = Process("Virtual", a_actor, a_faction, a_goToJail, a_removeStolenItems);
                return OLD_V(a_actor, a_faction, result.first, result.second);
            }
            static void NEW_C(RE::Actor* a_actor, RE::TESFaction* a_faction, bool a_goToJail, bool a_removeStolenItems) {
                auto result = Process("Console", a_actor, a_faction, a_goToJail, a_removeStolenItems);
                return OLD_C(a_actor, a_faction, result.first, result.second);
            }
            static void NEW_P(RE::Actor* a_actor, RE::TESFaction* a_faction, bool a_goToJail, bool a_removeStolenItems) {
                auto result = Process("Papyrus", a_actor, a_faction, a_goToJail, a_removeStolenItems);
                return OLD_P(a_actor, a_faction, result.first, result.second);
            }
            static inline REL::Relocation<decltype(NEW_V)> OLD_V;
            static inline REL::Relocation<decltype(NEW_C)> OLD_C;
            static inline REL::Relocation<decltype(NEW_P)> OLD_P;
        };
    public:
        static void Install() {
            // virtual > ID 452447 + 0x14A * 8 > ID 102836
            PlayerPayCrimeGold::OLD_V = REL::Relocation{ REL::ID(452447) }.write_vfunc(0x14A, PlayerPayCrimeGold::NEW_V);
            // console > ID 66511 + 0xB9 > call ID 102836
            PlayerPayCrimeGold::OLD_C = REL::Relocation{ REL::ID(66511), 0xB9 }.write_call<5>(PlayerPayCrimeGold::NEW_C);
            // papyrus > ID 117940 + 0x10 > jmp ID 102836
            PlayerPayCrimeGold::OLD_P = REL::Relocation{ REL::ID(117940), 0x10 }.write_jmp<5>(PlayerPayCrimeGold::NEW_P);
        }
    };

}

void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
    if (a_msg->type == SFSE::MessagingInterface::kPostLoad) {
        PPCGHook::PlayerPayCrimeGoldHook::Install();
    }
}

SFSE_PLUGIN_LOAD(const SFSE::LoadInterface* a_sfse) {
    SFSE::InitInfo info{
        .logPattern = "%d.%m.%Y %H:%M:%S [%s:%#] %v",
        .trampoline = true,
        .trampolineSize = 64
    };

    SFSE::Init(a_sfse, info);

    const auto gameInfo = a_sfse->RuntimeVersion().string(".");
    REX::INFO("Starfield v{}", gameInfo);

    const auto messagingInterface = SFSE::GetMessagingInterface();
    if (messagingInterface && messagingInterface->RegisterListener(MessageCallback)) {
        REX::INFO("Message listener registered");
    } else {
        REX::FAIL("Message listener not registered");
    }
    return true;

}
