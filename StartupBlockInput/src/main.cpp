namespace SBIUtility {

	bool error = false;
	bool locked = false;
	bool unlocked = false;

	std::uint32_t LowestFrameTime(std::uint32_t fallback) {
		auto dmData = []() {
			DEVMODEA result;
			auto size = sizeof(result);
			REL::WriteSafeFill(&result, 0, size);
			result.dmSize = size;
			return result;
		}();
		auto result = float(fallback);
		bool success = EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &dmData);
		if (success) {
			auto maxFrequency = float(dmData.dmDisplayFrequency);
			if (maxFrequency > 0.0F) result = 1000.0F / maxFrequency;
		}
		return std::uint32_t(std::max(result, 1.0F));
	}

	// 0 = ui error
	// 1 = already set
	// 2 = message error
	// 3 = success
	std::uint8_t ShowHideMenu(std::string menuName, bool newState, bool force) {
		if (auto sfui = RE::UI::GetSingleton(); sfui) {
			if (sfui->IsMenuOpen(menuName) == newState) {
				if (force) {
					auto sleepTime = LowestFrameTime(20);
					REX::INFO("Integer lowest frame time = {}", sleepTime);
					while (sfui->IsMenuOpen(menuName) == newState) Sleep(sleepTime);
				} else return 1;
			};
			if (auto msgQueue = RE::UIMessageQueue::GetSingleton(); msgQueue) {
				RE::BSFixedString fixedName = menuName;
				auto newMsg = newState ? RE::UI_MESSAGE_TYPE::kShow : RE::UI_MESSAGE_TYPE::kHide;
				auto msgHandle = msgQueue->AddMessage(fixedName, newMsg);
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
			auto result = ShowHideMenu("LoadingMenu", true, true);
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
			auto result = ShowHideMenu("LoadingMenu", false, false);
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
		REL::Relocation<T*> reloc{ address };
		return reloc.get();
	}

	RE::BSTEventSource<RE::MenuOpenCloseEvent>* GetMenuEventSource(RE::UI* ui) {
		using type = RE::BSTEventSource<RE::MenuOpenCloseEvent>;
		return GetMember<type>(ui, 0x20);
	}

	bool EndgameCredits() {
		auto ui = RE::UI::GetSingleton();
		return ui ? ui->IsMenuOpen("EndGameCreditsMenu") : false;
	}

}

namespace SBIConfig {

	std::uint32_t iDelayMS = 1000;
	bool bTimeOut = false;
	std::uint32_t iTimeOutMS = 120000;

	void ReadConfig() {
		// override defaults
		std::string filePath = "Data\\SFSE\\Plugins\\StartupBlockInput.ini";
		if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath)) {
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

	void LockThread() {
		SBIUtility::Lock("LockThread:"s + ThreadID);
	}

	void DelayThread(std::uint32_t sleepTime) {
		Sleep(sleepTime);
		SBIUtility::Unlock("DelayThread:"s + ThreadID);
	}

	void TimeoutThread(std::uint32_t sleepTime) {
		Sleep(sleepTime);
		SBIUtility::Unlock("TimeoutThread:"s + ThreadID);
	}

	class EventHandler final : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
	private:
		bool process = false;
		static EventHandler* GetSingleton() {
			static EventHandler self;
			return std::addressof(self);
		}
	public:
		static void Install(bool newState) {
			auto sfui = RE::UI::GetSingleton();
			auto handler = GetSingleton();
			if (sfui && handler) {
				if (handler->process == newState) return;
				auto source = SBIUtility::GetMenuEventSource(sfui);
				if (newState) source->RegisterSink(handler);
				else source->UnregisterSink(handler);
				handler->process = newState;
				REX::INFO("UI events sink {}registered / {:X}", handler->process ? "" : "un", (std::uint64_t)handler);
			} else REX::FAIL("Failed to obtain UI events sink");
		}
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) {
			if (process && a_event.menuName == "MainMenu" && a_event.opening) {
				std::thread(LockThread).detach();
				// setup time-out
				if (SBIConfig::bTimeOut) {
					REX::INFO("Lock called, creating TimeoutThread");
					std::thread(TimeoutThread, SBIConfig::iTimeOutMS).detach();
				}
				// unregister
				Install(false);
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	struct DataReloadHookReset {
		static void NEW(std::int64_t a1, const char* a2, const char* a3, const char* a4) {
			REX::INFO("Reload called, resetting switches");
			SBIUtility::error = false;
			SBIUtility::locked = false;
			SBIUtility::unlocked = false;
			return OLD(a1, a2, a3, a4);
		}
		inline static REL::THook OLD{ REL::ID(99468), 0x48, NEW };
	};

	struct DataReloadHookA {
		static void NEW(std::int64_t a1, std::int64_t a2) {
			if (!SBIUtility::EndgameCredits()) {
				std::thread(LockThread).detach();
				// setup time-out
				if (SBIConfig::bTimeOut) {
					REX::INFO("Lock called, creating TimeoutThread");
					std::thread(TimeoutThread, SBIConfig::iTimeOutMS).detach();
				}
			}
			return OLD(a1, a2);
		}
		inline static REL::THook OLD{ REL::ID(99468), 0x127A, NEW };
	};

	struct DataReloadHookB {
		static std::int64_t NEW() {
			if (!SBIUtility::EndgameCredits()) {
				REX::INFO("Data loaded, creating DelayThread");
				std::thread(DelayThread, SBIConfig::iDelayMS).detach();
			}
			return OLD();
		}
		inline static REL::THook OLD{ REL::ID(99468), 0x1987, NEW };
	};

	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
		if (a_msg->type == SFSE::MessagingInterface::kPostLoad) {
			// register for menus open/close
			EventHandler::Install(true);
		} else if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
			REX::INFO("Data loaded, creating DelayThread");
			std::thread(DelayThread, SBIConfig::iDelayMS).detach();
		} else return;
	}

}

SFSE_PLUGIN_LOAD(const SFSE::LoadInterface* a_sfse) {
	SFSE::Init(a_sfse, {
		.logPattern = "%d.%m.%Y %H:%M:%S [%s:%#] %v",
		.trampoline = true,
		.trampolineSize = 64
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
