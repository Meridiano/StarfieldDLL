namespace SBIUtility {

	bool locked = false;

	bool ShowHideMenu(std::string menuName, bool newState) {
		if (auto sfui = RE::UI::GetSingleton(); sfui) {
			if (sfui->IsMenuOpen(menuName) == newState) return true;
			if (auto msgQueue = RE::UIMessageQueue::GetSingleton(); msgQueue) {
				RE::UIMessage newMsg = newState ? RE::UIMessage::kShow : RE::UIMessage::kHide;
				auto msgHandler = msgQueue->AddMessage(menuName, newMsg);
				return (msgHandler != 0);
			}
		}
		return false;
	}

	void Lock(std::string source) {
		locked = ShowHideMenu("LoadingMenu", true);
		logs::info("SBIUtility.Lock({}) = {:X}", source, locked);
	}

	void Unlock(std::string source) {
		if (locked) {
			bool unlocked = ShowHideMenu("LoadingMenu", false);
			logs::info("SBIUtility.Unlock({}) = {:X}", source, unlocked);
			locked = !unlocked;
		} else logs::info("SBIUtility.Unlock({}) = Disabled", source);
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
		mINI::INIFile file("Data\\SFSE\\Plugins\\StartupBlockInput.ini");
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

	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
		if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
			logs::info("Data loaded, creating DelayThread");
			std::thread dt(DelayThread, SBIConfig::iDelayMS);
			dt.detach();
		} else return;
	}

	class EventHandler final:
		public RE::BSTEventSink<RE::MenuOpenCloseEvent>{
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
					std::thread tt(TimeoutThread, SBIConfig::iTimeOutMS);
					tt.detach();
				}
				// unregister
				auto sfui = RE::UI::GetSingleton();
				auto handler = EventHandler::GetSingleton();
				if (sfui && handler) sfui->UnregisterSink(handler);
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
	SFSE::Init(a_sfse);
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
	// register for menus open/close
	auto sfui = RE::UI::GetSingleton();
	auto handler = SBIProcess::EventHandler::GetSingleton();
	if (sfui && handler) {
		sfui->RegisterSink(handler);
		logs::info("UI events sink registered");
	} else {
		SFSE::stl::report_and_fail("UI events sink not registered");
	}
	SBIConfig::ReadConfig();
	// done
	return true;
}
