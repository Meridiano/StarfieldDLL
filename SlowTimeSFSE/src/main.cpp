namespace SlowTimeSettings {

	// general
	float fGlobalMult = 0.1F;
	float fPlayerMult = 1.0F;
	float fMouseMult = 1.0F;
	std::string sBlacklist = "";
	// hotkey
	std::int32_t iHotkey = 112;
	std::int32_t iModifier = 0;
	// message
	std::string sMessageOn = "";
	std::string sMessageOff = "";

	std::int32_t ConfigInt(mINI::INIStructure ini, std::string section, std::string key, std::int32_t fallback) {
		std::int32_t result = fallback;
		std::string raw = ini.get(section).get(key);
		try {
			result = std::stol(raw, nullptr, 0);
		} catch (...) {
			logs::info("Failed to read [{}]{} ini value", section, key);
		}
		logs::info("Int value [{}]{} is {}", section, key, result);
		return result;
	}

	float ConfigFloat(mINI::INIStructure ini, std::string section, std::string key, float fallback) {
		float result = fallback;
		std::string raw = ini.get(section).get(key);
		try {
			result = std::stof(raw, nullptr);
		} catch (...) {
			logs::info("Failed to read [{}]{} ini value", section, key);
		}
		logs::info("Float value [{}]{} is {}", section, key, result);
		return result;
	}

	std::string ConfigString(mINI::INIStructure ini, std::string section, std::string key, std::string fallback) {
		std::string result = fallback;
		if (ini.get(section).has(key)) result = ini.get(section).get(key);
		else logs::info("Failed to read [{}]{} ini value", section, key);
		logs::info("String value [{}]{} is \"{}\"", section, key, result);
		return result;
	}

	void LoadSettings() {
		mINI::INIFile file("Data\\SFSE\\Plugins\\SlowTimeSFSE.ini");
		mINI::INIStructure ini;
		if (file.read(ini)) {
			// general
			fGlobalMult = ConfigFloat(ini, "General", "fGlobalMult", 0.1F);
			fPlayerMult = ConfigFloat(ini, "General", "fPlayerMult", 1.0F);
			fMouseMult = ConfigFloat(ini, "General", "fMouseMult", 1.0F);
			sBlacklist = ConfigString(ini, "General", "sBlacklist", "");
			// hotkey
			iHotkey = ConfigInt(ini, "Hotkey", "iHotkey", 112);
			iModifier = ConfigInt(ini, "Hotkey", "iModifier", 0);
			// message
			sMessageOn = ConfigString(ini, "Message", "sMessageOn", "");
			sMessageOff = ConfigString(ini, "Message", "sMessageOff", "");
		} else logs::info("Config read error, all settings set to default");
	}

}

namespace SlowTimeUtility {

	class GameTimeMultiplier {
	private:
		std::vector<float*> GetPointers() {
			// easy part
			static REL::Relocation<float(*)> glo_1{REL::ID(763331)};
			static REL::Relocation<float(*)> glo_2{REL::ID(763332)};
			static REL::Relocation<float(*)> glo_3{REL::ID(891034), 0x18};
			static REL::Relocation<float(*)> pla_1{REL::ID(891034), 0x30};
			// hard part 1
			static REL::Relocation<std::uintptr_t(*)> ptr_1{REL::ID(891034), 0x10};
			std::uintptr_t glo_4_ptr = *ptr_1.get() + 0x4;
			std::uintptr_t pla_2_ptr = *ptr_1.get() + 0x8;
			static REL::Relocation<float(*)> glo_4{glo_4_ptr};
			static REL::Relocation<float(*)> pla_2{pla_2_ptr};
			// hard part 2
			static REL::Relocation<std::uintptr_t(*)> ptr_2{REL::ID(891034), 0x38};
			std::uintptr_t pla_3_ptr = *ptr_2.get() + 0x10;
			std::uintptr_t glo_5_ptr = *ptr_2.get() + 0x38;
			static REL::Relocation<float(*)> pla_3{pla_3_ptr};
			static REL::Relocation<float(*)> glo_5{glo_5_ptr};
			// done
			std::vector<float*> result{
				glo_1.get(), // 0
				glo_2.get(), // 1
				glo_3.get(), // 2
				pla_1.get(), // 3
				glo_4.get(), // 4
				pla_2.get(), // 5
				pla_3.get(), // 6
				glo_5.get()  // 7
			};
			return result;
		}
	public:
		void SetValues(float gloValue, float plaValue) {
			auto points = GetPointers();
			*points[0] = *points[1] = *points[2] = *points[4] = *points[7] = gloValue;
			*points[3] = *points[5] = *points[6] = plaValue;
		}
	};

	std::vector<std::string> Split(std::string input, char delim) {
		std::vector<std::string> result;
		std::stringstream stream(input);
		std::string item;
		while (std::getline(stream, item, delim)) result.push_back(item);
		return result;
	}

	bool IsBlacklistOpen(RE::UI* sfui) {
		std::vector<std::string> vBlacklist = Split(SlowTimeSettings::sBlacklist, '|');
		for (std::string menu : vBlacklist) if (sfui->IsMenuOpen(menu)) return true;
		return false;
	}

	void PlayWAV(std::string path) {
		bool result = PlaySound(path.data(), NULL, SND_FILENAME + SND_ASYNC);
		if (!result) logs::info("Error on sound play, path = \"{}\"", path);
	}

	bool CheckModifiers(bool alt, bool ctrl, bool shift) {
		bool altVal = (GetAsyncKeyState(VK_MENU) & 0x8000);
		if (altVal != alt) return false;
		bool ctrlVal = (GetAsyncKeyState(VK_CONTROL) & 0x8000);
		if (ctrlVal != ctrl) return false;
		bool shiftVal = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
		if (shiftVal != shift) return false;
		// all good
		return true;
	}

}

namespace SlowTimeProcess {

	bool bSlowTimeActive = false;
	float fMouseBackup = 0.0F;
	float fGamepadBackup = 0.0F;

	void ChangeMouseSpeed(int mode, float mult = 1.0F) {
		if (auto ini = RE::INIPrefSettingCollection::GetSingleton(); ini) {
			auto fMouse = ini->GetSetting("fMouseHeadingSensitivity:Controls");
			auto fGamepad = ini->GetSetting("fGamepadHeadingSensitivity:Controls");
			if (fMouse && fGamepad) {
				if (mode == 0) {
					// save backup
					fMouseBackup = fMouse->GetFloat();
					fGamepadBackup = fGamepad->GetFloat();
				} else if (mode == 1) {
					// save and apply
					ChangeMouseSpeed(0);
					fMouse->SetFloat(fMouse->GetFloat() * mult);
					fGamepad->SetFloat(fGamepad->GetFloat() * mult);
				} else if (mode == 2) {
					// restore backup
					fMouse->SetFloat(fMouseBackup);
					fGamepad->SetFloat(fGamepadBackup);
				}
			}
		}
	}
	
	void SetSlowTime(bool toggle) {
		float global = (toggle ? SlowTimeSettings::fGlobalMult : 1.0F);
		float player = (toggle ? SlowTimeSettings::fPlayerMult : 1.0F);
		float mouse = SlowTimeSettings::fMouseMult / SlowTimeSettings::fPlayerMult;
		SlowTimeUtility::GameTimeMultiplier GTM;
		GTM.SetValues(global, player);
		ChangeMouseSpeed(toggle ? 1 : 2, mouse);
		std::string sound = std::format("Data\\SFSE\\Plugins\\SlowTimeSFSE.{}.wav", toggle ? "On" : "Off");
		SlowTimeUtility::PlayWAV(sound);
	}

	void ToggleSlowTime() {
		if (auto sfui = RE::UI::GetSingleton(); sfui) {
			if (SlowTimeUtility::IsBlacklistOpen(sfui)) return;
			if (sfui->IsMenuOpen("HUDMessagesMenu")) {
				bSlowTimeActive = !bSlowTimeActive;
				SetSlowTime(bSlowTimeActive);
				auto message = (bSlowTimeActive ? SlowTimeSettings::sMessageOn : SlowTimeSettings::sMessageOff);
				if (message.size() > 0) RE::DebugNotification(message.data());
			}
		}
	}
	
	void HotkeyThread(std::uint32_t sleepTime) {
		bool keyListener = true;
		bool altProc = (SlowTimeSettings::iModifier & 1);
		bool ctrlProc = (SlowTimeSettings::iModifier & 2);
		bool shiftProc = (SlowTimeSettings::iModifier & 4);
		while (true) {
			bool keyPressed = (GetAsyncKeyState(SlowTimeSettings::iHotkey) & 0x8000);
			if (keyPressed) {
				if (keyListener) {
					keyListener = false;
					bool toggle = SlowTimeUtility::CheckModifiers(altProc, ctrlProc, shiftProc);
					if (toggle) ToggleSlowTime();
				}
			} else keyListener = true;
			Sleep(sleepTime);
		}
	}

	class EventHandler:
	public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
	public:
		static EventHandler* GetSingleton() {
			static EventHandler self;
			auto address = std::addressof(self);
			logs::info("Event handler called, address = {:X}", (std::uint64_t)address);
			return address;
		}
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) {
			if (SlowTimeProcess::bSlowTimeActive && a_event.opening) {
				std::vector<std::string> vBlacklist = SlowTimeUtility::Split(SlowTimeSettings::sBlacklist, '|');
				auto b = vBlacklist.begin(); auto e = vBlacklist.end();
				bool found = std::find(b, e, a_event.menuName) != e;
				if (found) {
					SlowTimeProcess::bSlowTimeActive = false;
					SlowTimeProcess::SetSlowTime(false);
				}
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
		if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
			// register for key
			std::thread hotkey(HotkeyThread, 50);
			hotkey.detach();
			// register for menu
			auto sfui = RE::UI::GetSingleton();
			auto handler = EventHandler::GetSingleton();
			if (sfui && handler) {
				sfui->RegisterSink(handler);
				logs::info("Menu listener registered");
			} else logs::info("Menu listener not registered");
			// save mouse values
			ChangeMouseSpeed(0);
		} else return;
	}

}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {

	SFSE::Init(a_sfse);
	const auto pluginInfo = SFSE::PluginVersionData::GetSingleton();
	logs::info(
		"{} version {} is loading into Starfield {}",
		std::string(pluginInfo->pluginName),
		REL::Version::unpack(pluginInfo->pluginVersion).string("."),
		a_sfse->RuntimeVersion().string(".")
	);
	SFSE::AllocTrampoline(128);

	// read settings
	SlowTimeSettings::LoadSettings();

	// register stuff
	const auto SFSEMessagingInterface = SFSE::GetMessagingInterface();
	if (SFSEMessagingInterface && SFSEMessagingInterface->RegisterListener(SlowTimeProcess::MessageCallback)) {
		logs::info("Message listener registered");
	} else {
		logs::info("Message listener not registered");
		return false;
	}
	return true;
}
