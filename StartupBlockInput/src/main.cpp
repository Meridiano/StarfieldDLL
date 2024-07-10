namespace SBIUtility {

	STARTUPINFO si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pi;
	LPCTSTR path = "Data\\SFSE\\Plugins\\StartupBlockInput.exe";
	BOOL success = false;

	void LOCK() {
		success = CreateProcessA(
			path,              // The path
			NULL,              // Command line
			NULL,              // Process handle not inheritable
			NULL,              // Thread handle not inheritable
			FALSE,             // Set handle inheritance to FALSE
			NULL,              // No creation flags
			NULL,              // Use parent's environment block
			NULL,              // Use parent's starting directory
			std::addressof(si),// Pointer to STARTUPINFO structure
			std::addressof(pi) // Pointer to PROCESS_INFORMATION structure
		);
		logs::info("SBIUtility.LOCK = {}", success);
	}

	void UNLOCK() {
		if (success) {
			BOOL a = TerminateProcess(pi.hProcess, 0);
			BOOL b = TerminateProcess(pi.hThread, 0);
			BOOL c = CloseHandle(pi.hProcess);
			BOOL d = CloseHandle(pi.hThread);
			logs::info("SBIUtility.UNLOCK = {}|{}|{}|{}", a, b, c, d);
		} else logs::info("SBIUtility.UNLOCK = Disabled");
	}

	std::uint32_t UInt32MinMax(std::uint32_t val, std::uint32_t min, std::uint32_t max) {
		if (val < min) return min;
		if (val > max) return max;
		return val;
	}

}

namespace SBIProcess {

	void DelayThread(std::uint32_t sleepTime) {
		Sleep(sleepTime);
		SBIUtility::UNLOCK();
	}

	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
		if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
			// define delay
			std::uint32_t delay = 0;
			// read delay
			mINI::INIFile file("Data\\SFSE\\Plugins\\StartupBlockInput.ini");
			mINI::INIStructure ini;
			if (file.read(ini)) {
				try {
					std::string raw = ini.get("General").get("iDelayMS");
					delay = std::stol(raw, nullptr, 0);
					delay = SBIUtility::UInt32MinMax(delay, 1, 65535);
				} catch (...) { delay = 1000; }
			} else { delay = 1000; }
			logs::info("Data loaded, iDelayMS = {}", delay);
			std::thread dt(DelayThread, delay);
			dt.detach();
		} else return;
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
			bool matchName = stricmp(a_event.menuName.data(), "FaderMenu") == 0;
			bool matchOpen = a_event.opening;
			// process lock
			if (matchName && matchOpen) {
				SBIUtility::LOCK();
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
	// done
	return true;
}
