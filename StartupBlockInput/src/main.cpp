namespace SBIUtility {

	bool error = false;
	bool locked = false;
	bool unlocked = false;

	RE::UIMessageQueue* GetMessageQueue() {
		using type = std::invoke_result_t<decltype(GetMessageQueue)>;
		static REL::Relocation<type*> singleton{ REL::ID(937897) };
		return *singleton;
	}

	std::int64_t AddMessageToQueue(RE::UIMessageQueue* queue, RE::BSFixedString* menuName, RE::UI_MESSAGE_TYPE msgType) {
		using func_t = decltype(&AddMessageToQueue);
		static REL::Relocation<func_t> func{ REL::ID(130659) };
		return func(queue, menuName, msgType);
	}

	// 0 = ui error
	// 1 = already set
	// 2 = message error
	// 3 = success
	std::uint8_t ShowHideMenu(std::string menuName, bool newState) {
		if (auto sfui = RE::UI::GetSingleton(); sfui) {
			if (sfui->IsMenuOpen(menuName) == newState) return 1;
			if (auto msgQueue = GetMessageQueue(); msgQueue) {
				RE::BSFixedString fixedName = menuName;
				auto newMsg = newState ? RE::UI_MESSAGE_TYPE::kShow : RE::UI_MESSAGE_TYPE::kHide;
				auto msgHandle = AddMessageToQueue(msgQueue, &fixedName, newMsg);
				return (msgHandle == 0 ? 2 : 3);
			}
		}
		return 0;
	}

	void Lock(std::string source) {
		if (error) {
			REX::INFO("SBIUtility.Lock({}) = Error", source);
		} else if (unlocked) {
			REX::INFO("SBIUtility.Lock({}) = Disabled", source);
		} else {
			auto result = ShowHideMenu("LoadingMenu", true);
			REX::INFO("SBIUtility.Lock({}) = {}", source, result);
			if (result == 3) locked = true;
			else error = true;
		}
	}

	void Unlock(std::string source) {
		if (error) {
			REX::INFO("SBIUtility.Unlock({}) = Error", source);
		} else if (unlocked) {
			REX::INFO("SBIUtility.Unlock({}) = Disabled", source);
		} else {
			auto result = ShowHideMenu("LoadingMenu", false);
			REX::INFO("SBIUtility.Unlock({}) = {}", source, result);
			if (result == 3) unlocked = true;
			else error = true;
		}
	}

	std::uint32_t UInt32MinMax(std::uint32_t val, std::uint32_t min, std::uint32_t max) {
		if (val < min) return min;
		if (val > max) return max;
		return val;
	}

	template <typename T>
	T* GetMember(const void* base, std::ptrdiff_t offset) {
		auto address = std::uintptr_t(base) + offset;
		auto reloc = REL::Relocation<T*>(address);
		return reloc.get();
	};

	RE::BSTEventSource<RE::MenuOpenCloseEvent>* GetMenuEventSource(RE::UI* ui) {
		using type = RE::BSTEventSource<RE::MenuOpenCloseEvent>;
		return GetMember<type>(ui, 0x20);
	}

	std::string GetThreadID() {
		return std::to_string(
			std::bit_cast<std::uint32_t>(
				std::this_thread::get_id()
			)
		);
	}

}

namespace SBIConfig {

	std::uint32_t iDelayMS = 1000;
	bool bTimeOut = false;
	std::uint32_t iTimeOutMS = 120000;

	void ReadConfig() {
		// override defaults
		std::string filePath = "Data\\SFSE\\Plugins\\StartupBlockInput.ini";
		if (std::filesystem::exists(filePath)) {
			mINI::INIFile file(filePath);
			if (mINI::INIStructure ini; file.read(ini)) {
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
					REX::INFO("ReadConfig > Conversion issue");
				}
			} else {
				REX::INFO("ReadConfig > Ini structure issue");
			}
		} else {
			REX::INFO("ReadConfig > File path issue");
		}
		REX::INFO("SBIConfig.iDelayMS = {}", iDelayMS);
		REX::INFO("SBIConfig.bTimeOut = {:X}", bTimeOut);
		REX::INFO("SBIConfig.iTimeOutMS = {}", iTimeOutMS);
	}

}

namespace SBIProcess {

	bool process = true;

	void LockThread() {
		SBIUtility::Lock("LockThread:"s + SBIUtility::GetThreadID());
	}

	void DelayThread(std::uint32_t sleepTime) {
		Sleep(sleepTime);
		SBIUtility::Unlock("DelayThread:"s + SBIUtility::GetThreadID());
	}

	void TimeoutThread(std::uint32_t sleepTime) {
		Sleep(sleepTime);
		SBIUtility::Unlock("TimeoutThread:"s + SBIUtility::GetThreadID());
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
			bool match = process && (a_event.menuName == "MainMenu") && a_event.opening;
			// process lock
			if (match) {
				LockThread();
				// setup time-out
				if (SBIConfig::bTimeOut) {
					REX::INFO("Lock called, creating TimeoutThread");
					std::thread(TimeoutThread, SBIConfig::iTimeOutMS).detach();
				}
				process = false;
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
				SBIUtility::GetMenuEventSource(sfui)->RegisterSink(handler);
				REX::INFO("UI events sink registered / {:X}", (std::uint64_t)handler);
			} else {
				REX::FAIL("Failed to register UI events sink");
			}
		} else if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
			REX::INFO("Data loaded, creating DelayThread");
			std::thread(DelayThread, SBIConfig::iDelayMS).detach();
		} else return;
	}

}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
	SFSE::Init(a_sfse, {
		.logPattern = "%d.%m.%Y %H:%M:%S [%s:%#] %v"
	});
	// show base info
	const auto gameInfo = a_sfse->RuntimeVersion().string(".");
	REX::INFO("Starfield v{}", gameInfo);
	// register for sfse message
	const auto messagingInterface = SFSE::GetMessagingInterface();
	if (messagingInterface && messagingInterface->RegisterListener(SBIProcess::MessageCallback)) {
		REX::INFO("Message listener registered");
	} else {
		REX::FAIL("Message listener not registered");
	}
	// done
	SBIConfig::ReadConfig();
	return true;
}
