namespace PPCGUtility {

    void ConsoleExecute(std::string command) {
        static REL::Relocation<void(**)> BGSScaleFormManager{ REL::ID(879512) };
        static REL::Relocation<void(*)(void*, const char*)> ExecuteCommand{ REL::ID(166307) };
        ExecuteCommand(*BGSScaleFormManager, command.data());
        logs::info("Console execution [ {} ]", command);
    }

    void SetCrimeGold(RE::Actor* actor, RE::TESFaction* faction, bool violent, std::int32_t value) {
        static REL::Relocation<decltype(&SetCrimeGold)> SetCrimeGoldNative{ REL::ID(153727) };
        return SetCrimeGoldNative(actor, faction, violent, value);
    }

}

namespace PPCGHook {

    class PlayerPayCrimeGoldHook {
    private:
        struct PlayerPayCrimeGoldCall {
            static void thunk(RE::Actor* a_actor, RE::TESFaction* a_faction, bool a_goToJail, bool a_removeStolenItems) {
                bool goToJail = a_goToJail;
                bool removeStolenItems = a_removeStolenItems;
                bool payCrimeGold = true;
                // get faction id
                std::uint32_t factionID = a_faction->formID;
                logs::info("PlayerPayCrimeGold called, faction FormID is {:X}, processing", factionID);
                // load ini
                CSimpleIniA ini;
                ini.SetUnicode();
                SI_Error iniResult = ini.LoadFile("Data\\SFSE\\Plugins\\PlayerPayCrimeGoldTweak.ini");
                if (iniResult < 0) logs::info("Settings file not found, input values: {} / {} / {}", goToJail, removeStolenItems, payCrimeGold);
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
                    logs::info("Settings file parsed, result values: {} / {} / {}", goToJail, removeStolenItems, payCrimeGold);
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
                // call original function
                return func(a_actor, a_faction, goToJail, removeStolenItems);
            }
            static inline REL::Relocation<decltype(thunk)> func;
            static inline std::size_t idx = 0x14B;
        };
    public:
        static void Install() {
            // virtual > ID 423292 + 0x14B * 8 > ID 153606
            SFSE::stl::write_vfunc<PlayerPayCrimeGoldCall>(REL::ID(423292));
            // console > ID 110020 + 0xB9 > call ID 153606
            const REL::Relocation console{ REL::ID(110020), 0xB9 };
            SFSE::stl::write_thunk_call<PlayerPayCrimeGoldCall>(console.address());
            // papyrus > ID 171509 + 0x10 > jmp ID 153606
            const REL::Relocation papyrus{ REL::ID(171509), 0x10 };
            SFSE::stl::write_thunk_jump<PlayerPayCrimeGoldCall>(papyrus.address());
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
    SFSE::Init(a_sfse, false);
    SFSE::AllocTrampoline(256);

    logs::init();
    spdlog::set_pattern("%d.%m.%Y %H:%M:%S [%s:%#] %v");

    const auto pluginInfo = SFSE::PluginVersionData::GetSingleton();
    logs::info(
        "{} version {} is loading into Starfield {}",
        std::string(pluginInfo->pluginName),
        REL::Version::unpack(pluginInfo->pluginVersion).string("."),
        a_sfse->RuntimeVersion().string(".")
    );

    const auto SFSEMessagingInterface = SFSE::GetMessagingInterface();
    if (SFSEMessagingInterface && SFSEMessagingInterface->RegisterListener(PPCGProcess::MessageCallback)) {
        logs::info("Message listener registered");
    } else {
        SFSE::stl::report_and_fail("Message listener not registered");
    }
    return true;

}
