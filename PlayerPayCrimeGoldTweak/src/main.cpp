namespace PPCGUtility {

    void ConsoleExecute(std::string command) {
        std::thread([](std::string commandLambda) {
            static REL::Relocation<void*> UnknownManager{ REL::ID(949606) };
            static REL::Relocation<void(*)(void*, const char*)> ExecuteCommand{ REL::ID(113576) };
            ExecuteCommand(UnknownManager.get(), commandLambda.data());
        }, command).detach();
    }

    void SetCrimeGold(RE::Actor* actor, RE::TESFaction* faction, bool violent, std::int32_t value) {
        static REL::Relocation<decltype(&SetCrimeGold)> SetCrimeGoldNative{ REL::ID(102824) };
        return SetCrimeGoldNative(actor, faction, violent, value);
    }

}

namespace PPCGHook {

    std::pair<bool, bool> Process(RE::Actor* a_actor, RE::TESFaction* a_faction, bool a_goToJail, bool a_removeStolenItems, std::string a_source) {
        bool goToJail = a_goToJail;
        bool removeStolenItems = a_removeStolenItems;
        bool payCrimeGold = true;
        // get faction id
        std::uint32_t factionID = a_faction->formID;
        REX::INFO("PlayerPayCrimeGold.{} called, faction FormID is {:X}, processing", a_source, factionID);
        // load ini
        CSimpleIniA ini;
        ini.SetUnicode();
        SI_Error iniResult = ini.LoadFile("Data\\SFSE\\Plugins\\PlayerPayCrimeGoldTweak.ini");
        if (iniResult < 0) REX::INFO("Settings file not found, input values: {} / {} / {}", goToJail, removeStolenItems, payCrimeGold);
        else {
            // general
            int vanillaFactions = ini.GetLongValue("General", "VanillaFactions", 0);
            // modes
            int goToJailMode = ini.GetLongValue("Modes", "GoToJailMode", 0);
            int removeStolenItemsMode = ini.GetLongValue("Modes", "RemoveStolenItemsMode", 0);
            int payCrimeGoldMode = ini.GetLongValue("Modes", "PayCrimeGoldMode", 0);
            // values
            bool goToJailValue = ini.GetBoolValue("Values", "GoToJailValue", false);
            bool removeStolenItemsValue = ini.GetBoolValue("Values", "RemoveStolenItemsValue", false);
            bool payCrimeGoldValue = ini.GetBoolValue("Values", "PayCrimeGoldValue", false);
            // check faction id
            bool factionIsVanilla = (factionID >> 24) <= vanillaFactions;
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
            REX::INFO("Settings file parsed, result values: {} / {} / {}", goToJail, removeStolenItems, payCrimeGold);
        }
        // allow landing if actor is in space
        if (a_actor->IsInSpace(true) && !goToJail) {
            std::string command = std::format("CGF \"{}.{}\"", "PlayerPayCrimeGoldTweak", "AllowLanding");
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
                auto result = Process(a_actor, a_faction, a_goToJail, a_removeStolenItems, "Virtual");
                return OLD_V(a_actor, a_faction, result.first, result.second);
            }
            static void NEW_C(RE::Actor* a_actor, RE::TESFaction* a_faction, bool a_goToJail, bool a_removeStolenItems) {
                auto result = Process(a_actor, a_faction, a_goToJail, a_removeStolenItems, "Console");
                return OLD_C(a_actor, a_faction, result.first, result.second);
            }
            static void NEW_P(RE::Actor* a_actor, RE::TESFaction* a_faction, bool a_goToJail, bool a_removeStolenItems) {
                auto result = Process(a_actor, a_faction, a_goToJail, a_removeStolenItems, "Papyrus");
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

namespace PPCGProcess {

    void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
        if (a_msg->type == SFSE::MessagingInterface::kPostLoad) {
            PPCGHook::PlayerPayCrimeGoldHook::Install();
        } else return;
    }

}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
    SFSE::InitInfo info{
        .logPattern = "%d.%m.%Y %H:%M:%S [%s:%#] %v",
        .trampoline = true,
        .trampolineSize = 64
    };

    SFSE::Init(a_sfse, info);

    const auto gameInfo = a_sfse->RuntimeVersion().string(".");
    REX::INFO("Starfield v{}", gameInfo);

    const auto messagingInterface = SFSE::GetMessagingInterface();
    if (messagingInterface && messagingInterface->RegisterListener(PPCGProcess::MessageCallback)) {
        REX::INFO("Message listener registered");
    } else {
        REX::FAIL("Message listener not registered");
    }
    return true;

}
