static std::vector<std::string>                                        dataLoadedMap;
static std::map<int, std::vector<std::string>>                         gameLoadedMap;
static std::map<std::pair<std::string, int>, std::vector<std::string>> menuActionMap;

static bool saveLoaded = false;

namespace CCRUtility {

    void ConsoleExecute(std::string command) {
        static REL::Relocation<void(**)> BGSScaleFormManager{REL::ID(879512)};
        static REL::Relocation<void(*)(void*, const char*)> ExecuteCommand{REL::ID(166307)};
        ExecuteCommand(*BGSScaleFormManager, command.data());
    }

    RE::TESObjectCELL* GetParentCell(void* a1, void* a2, RE::TESObjectREFR* a3) {
        using func_t = decltype(&GetParentCell);
        REL::Relocation<func_t> func{REL::ID(172297)};
        return func(a1, a2, a3);
    }

    std::uint32_t PlayerCellFormID() {
        auto pc = RE::PlayerCharacter::GetSingleton();
        if (pc) {
            auto cell = GetParentCell(nullptr, nullptr, pc);
            if (cell) return (cell->formID);
        }
        return 0;
    }

    int GameLoadType() {
        if (saveLoaded) return 2;
        if (PlayerCellFormID() == 0) return -1;
        auto gameDaysPassed = RE::TESForm::LookupByID<RE::TESGlobal>(0x39);
        if (gameDaysPassed && gameDaysPassed->GetValue() < 0.334F) return 1;
        return -1;
    }

}

namespace CCRFunctions {

    void StoreCommands() {
        std::string path = "Data/SFSE/Plugins/ConsoleCommandRunner";
        if (fs::exists(path)) {
            std::string type = ".toml";
            for (fs::directory_entry fileEntry : fs::directory_iterator(path)) {
                fs::path filePath = fileEntry.path();
                if (fs::is_regular_file(filePath) && filePath.extension() == type) {
                    auto fileName = filePath.filename().string();
                    logs::info("Reading config file >> {}", fileName);
                    toml::parse_result data;
                    try {
                        data = toml::parse_file(filePath.string());
                    } catch (...) {
                        logs::info("Parsing error, file name >> {}", fileName);
                        continue;
                    }
                    toml::array* events = data.get_as<toml::array>("Event");
                    if (events) for (toml::node& event : *events) {
                        toml::table eventTable = *event.as_table();
                        std::string eventType = eventTable["EventType"].value_or<std::string>("");
                        if (eventType == "DataLoaded") {
                            toml::array* comArray = eventTable["Commands"].as_array();
                            if (comArray) for (toml::node& com : *comArray) {
                                std::string command = com.as_string()->get();
                                logs::info("{} store >> {}", eventType, command);
                                dataLoadedMap.push_back(command);
                            }
                        } else if (eventType == "GameLoaded") {
                            int loadType = 0;
                            auto loadTypeData = eventTable["aiLoadType"].as_integer();
                            if (loadTypeData) loadType = loadTypeData->get();
                            toml::array* comArray = eventTable["Commands"].as_array();
                            if (comArray) for (toml::node& com : *comArray) {
                                std::string command = com.as_string()->get();
                                logs::info("{} with options [ {} ] store >> {}", eventType, loadType, command);
                                if (gameLoadedMap.count(loadType)) gameLoadedMap.at(loadType).push_back(command);
                                else gameLoadedMap.insert_or_assign(loadType, std::vector(1, command));
                            }
                        } else if (eventType == "OnMenuOpenCloseEvent") {
                            std::string menuName = "";
                            int isOpening = -1;
                            auto menuNameData = eventTable["asMenuName"].as_string();
                            auto isOpeningData = eventTable["abOpening"].as_boolean();
                            if (menuNameData) menuName = menuNameData->get();
                            if (isOpeningData) isOpening = isOpeningData->get();
                            std::pair menuAction = std::make_pair(menuName, isOpening);
                            toml::array* comArray = eventTable["Commands"].as_array();
                            if (comArray) for (toml::node& com : *comArray) {
                                std::string command = com.as_string()->get();
                                logs::info("{} with options [ {} {} ] store >> {}", eventType, menuName, isOpening, command);
                                if (menuActionMap.count(menuAction)) menuActionMap.at(menuAction).push_back(command);
                                else menuActionMap.insert_or_assign(menuAction, std::vector(1, command));
                            }
                        }
                    }
                }
            }
        }
    }

    void RunDataCommands() {
        for (std::string command : dataLoadedMap) {
            if (command.empty()) continue;
            logs::info("{} execute >> {}", "DataLoaded", command);
            CCRUtility::ConsoleExecute(command);
        }
    }

    void RunGameCommands(int loadType) {
        if (gameLoadedMap.count(loadType)) {
            auto commandArray = gameLoadedMap.at(loadType);
            for (std::string command : commandArray) {
                if (command.empty()) continue;
                logs::info("{} with options [ {} ] execute >> {}", "GameLoaded", loadType, command);
                CCRUtility::ConsoleExecute(command);
            }
        }
    }

    void RunMenuCommands(std::string sName, int iOpen) {
        for (auto mapEntry : menuActionMap) {
            auto menuAction = mapEntry.first;
            std::string menuName = menuAction.first;
            int isOpening = menuAction.second;
            bool matchName = (menuName == sName || menuName == "");
            bool matchOpening = (isOpening == iOpen || isOpening == -1);
            if (matchName && matchOpening) {
                auto commandArray = mapEntry.second;
                for (std::string command : commandArray) {
                    if (command.empty()) continue;
                    logs::info("{} with options [ {} {} ] execute >> {}", "OnMenuOpenCloseEvent", menuName, isOpening, command);
                    CCRUtility::ConsoleExecute(command);
                }
            }
        }
    }

}

namespace CCRHooks {

    class EventHandler final:
        public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
        // variables
        std::string sName; int iOpen;
    public:
        static EventHandler* GetSingleton() {
            static EventHandler self;
            auto address = std::addressof(self);
            logs::info("Event handler called, address = {:X}", (std::uint64_t)address);
            return address;
        }
        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) {
            // assign
            sName = a_event.menuName;
            iOpen = a_event.opening;
            // process
            if (sName == "HUDMessagesMenu" && iOpen == 1) {
                auto loadType = CCRUtility::GameLoadType();
                if (loadType != -1) {
                    saveLoaded = false;
                    CCRFunctions::RunGameCommands(0);
                    CCRFunctions::RunGameCommands(loadType);
                }
            }
            CCRFunctions::RunMenuCommands(sName, iOpen);
            return RE::BSEventNotifyControl::kContinue;
        }
    };

    class LoadGameHook {
    public:
        static void Install() {
            static REL::Relocation<std::uintptr_t> pcVTable{REL::ID(423292)};
            OriginalLoadGame = pcVTable.write_vfunc(0x1B, ModifiedLoadGame);
        }
    private:
        static std::int64_t ModifiedLoadGame(RE::PlayerCharacter* a1, RE::BGSLoadFormBuffer* a2) {
            saveLoaded = true;
            return OriginalLoadGame(a1, a2);
        }
        static inline REL::Relocation<decltype(ModifiedLoadGame)> OriginalLoadGame;
    };

    void InstallHooks() {
        LoadGameHook::Install();
    }

}

namespace CCRProcess {
	
	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
		if (a_msg->type == SFSE::MessagingInterface::kPostLoad) {
			CCRFunctions::StoreCommands();
            CCRHooks::InstallHooks();
        } else if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
            CCRFunctions::RunDataCommands();
            auto sfui = RE::UI::GetSingleton();
            auto handler = CCRHooks::EventHandler::GetSingleton();
            if (sfui && handler) sfui->RegisterSink(handler);
        } else return;
	}

}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse)
{
	SFSE::Init(a_sfse);
    SFSE::AllocTrampoline(64);

	const auto pluginInfo = SFSE::PluginVersionData::GetSingleton();
	logs::info(
		"{} version {} is loading into Starfield {}",
		std::string(pluginInfo->pluginName),
		REL::Version::unpack(pluginInfo->pluginVersion).string("."),
		a_sfse->RuntimeVersion().string(".")
	);

	const auto SFSEMessagingInterface = SFSE::GetMessagingInterface();
	if (SFSEMessagingInterface && SFSEMessagingInterface->RegisterListener(CCRProcess::MessageCallback)) {
		logs::info("Message listener registered");
	} else {
		SFSE::stl::report_and_fail("Message listener not registered");
	}
	return true;

}
