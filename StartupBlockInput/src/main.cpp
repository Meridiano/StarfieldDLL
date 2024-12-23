namespace SBIUtility {

	bool error = false;
	bool locked = false;
	bool unlocked = false;

	// 0 = ui error
	// 1 = already set
	// 2 = message error
	// 3 = success
	std::uint8_t ShowHideMenu(std::string menuName, bool newState) {
		if (auto sfui = RE::UI::GetSingleton(); sfui) {
			if (sfui->IsMenuOpen(menuName) == newState) return 1;
			if (auto msgQueue = RE::UIMessageQueue::GetSingleton(); msgQueue) {
				auto newMsg = newState ? RE::UI_MESSAGE_TYPE::kShow : RE::UI_MESSAGE_TYPE::kHide;
				auto msgHandle = msgQueue->AddMessage(menuName, newMsg);
				return (msgHandle == 0 ? 2 : 3);
			}
		}
		return 0;
	}

	void Lock(std::string source) {
		if (error) {
			logs::info("SBIUtility.Lock({}) = Error", source);
		} else if (unlocked) {
			logs::info("SBIUtility.Lock({}) = Disabled", source);
		} else {
			auto result = ShowHideMenu("LoadingMenu", true);
			logs::info("SBIUtility.Lock({}) = {}", source, result);
			if (result == 3) locked = true;
			else error = true;
		}
	}

	void Unlock(std::string source) {
		if (error) {
			logs::info("SBIUtility.Unlock({}) = Error", source);
		} else if (unlocked) {
			logs::info("SBIUtility.Unlock({}) = Disabled", source);
		} else {
			auto result = ShowHideMenu("LoadingMenu", false);
			logs::info("SBIUtility.Unlock({}) = {}", source, result);
			if (result == 3) unlocked = true;
			else error = true;
		}
	}

	std::uint32_t UInt32MinMax(std::uint32_t val, std::uint32_t min, std::uint32_t max) {
		if (val < min) return min;
		if (val > max) return max;
		return val;
	}

}

namespace SBIConfig {

	std::uint32_t iDelayMS;
	bool bTimeOut;
	std::uint32_t iTimeOutMS;

	void ReadConfig() {
		// set default
		iDelayMS = 1000;
		bTimeOut = false;
		iTimeOutMS = 120000;
		// override
		std::string filePath = "Data\\SFSE\\Plugins\\StartupBlockInput.ini";
		if (std::filesystem::exists(filePath)) {
			mINI::INIFile file(filePath);
			mINI::INIStructure ini;
			if (file.read(ini)) {
				std::string raw;
				try {
					// iDelayMS
					raw = ini.get("General").get("iDelayMS");
					iDelayMS = std::stol(raw, nullptr, 0);
					iDelayMS = SBIUtility::UInt32MinMax(iDelayMS, 1, 65535);
					// bTimeout
					raw = ini.get("General").get("bTimeOut");
					bTimeOut = (std::stol(raw, nullptr, 0) != 0);
					// iTimeOutMS
					raw = ini.get("General").get("iTimeOutMS");
					iTimeOutMS = std::stol(raw, nullptr, 0);
					iTimeOutMS = SBIUtility::UInt32MinMax(iTimeOutMS, 5000, 600000);
				} catch (...) {
					logs::info("ReadConfig > Conversion issue");
				}
			} else {
				logs::info("ReadConfig > Ini structure issue");
			}
		} else {
			logs::info("ReadConfig > File path issue");
		}
		logs::info("SBIConfig.iDelayMS = {}", iDelayMS);
		logs::info("SBIConfig.bTimeOut = {:X}", bTimeOut);
		logs::info("SBIConfig.iTimeOutMS = {}", iTimeOutMS);
	}

}

namespace SBIProcess {

	void DelayThread(std::uint32_t sleepTime) {
		Sleep(sleepTime);
		SBIUtility::Unlock("DelayThread");
	}

	void TimeoutThread(std::uint32_t sleepTime) {
		Sleep(sleepTime);
		SBIUtility::Unlock("TimeoutThread");
	}

	class EventHandler final:
		public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
	public:
		static EventHandler* GetSingleton() {
			static EventHandler self;
			return std::addressof(self);
		}
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) {
			// resolve menu
			bool match = (a_event.menuName == "MainMenu") && a_event.opening;
			// process lock
			if (match) {
				SBIUtility::Lock("EventHandler");
				// setup time-out
				if (SBIConfig::bTimeOut) {
					logs::info("Lock called, creating TimeoutThread");
					std::thread(TimeoutThread, SBIConfig::iTimeOutMS).detach();
				}
				// unregister
				auto sfui = RE::UI::GetSingleton();
				if (sfui && this) {
					sfui->UnregisterSink(this);
					logs::info("UI events sink unregistered / {:X}", (std::uint64_t)this);
				} else {
					SFSE::stl::report_and_fail("Failed to unregister UI events sink");
				}
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
		if (a_msg->type == SFSE::MessagingInterface::kPostLoad) {
			// register for menus open/close
			auto sfui = RE::UI::GetSingleton();
			auto handler = SBIProcess::EventHandler::GetSingleton();
			if (sfui && handler) {
				sfui->RegisterSink(handler);
				logs::info("UI events sink registered / {:X}", (std::uint64_t)handler);
			} else {
				SFSE::stl::report_and_fail("Failed to register UI events sink");
			}
		} else if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
			logs::info("Data loaded, creating DelayThread");
			std::thread(DelayThread, SBIConfig::iDelayMS).detach();
		} else return;
	}

}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
	SFSE::Init(a_sfse, false);
	logs::init();
	spdlog::set_pattern("%d.%m.%Y %H:%M:%S [%s:%#] %v");
	// show base info
	const auto pluginInfo = SFSE::PluginVersionData::GetSingleton();
	logs::info(
		"{} version {} is loading into Starfield {}",
		std::string(pluginInfo->pluginName),
		REL::Version::unpack(pluginInfo->pluginVersion).string("."),
		a_sfse->RuntimeVersion().string(".")
	);
	// register for sfse message
	const auto sfseMessagingInterface = SFSE::GetMessagingInterface();
	if (sfseMessagingInterface && sfseMessagingInterface->RegisterListener(SBIProcess::MessageCallback)) {
		logs::info("Message listener registered");
	} else {
		SFSE::stl::report_and_fail("Message listener not registered");
	}
	// done
	SBIConfig::ReadConfig();
	return true;
}
