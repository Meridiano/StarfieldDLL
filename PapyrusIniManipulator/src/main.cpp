#include "hpp/papyrus.hpp"
#include "hpp/console.hpp"

void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
	if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
		std::string message = PIMConsole::Register() ? "installed" : "not installed";
		logs::info("Console commands {}", message);
	} else return;
}

class PapyrusHook {
private:
	struct PapyrusCall {
		static void thunk(RE::BSScript::IVirtualMachine** a_vm) {
			// call original
			func(a_vm);
			// register new
			PIMPapyrus::RegisterFunctions(*a_vm);
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};
public:
	static void Install() {
		static auto target = REL::Relocation(REL::ID(169912), 0x514).get();
		if (*std::bit_cast<std::uint8_t*>(target) != 0xE8) throw std::exception("incorrect opcode");
		SFSE::stl::write_thunk_call<PapyrusCall>(target);
	}
};

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
	SFSE::Init(a_sfse, false);

	logs::init();
	spdlog::set_pattern("%d.%m.%Y %H:%M:%S [%s:%#] %v");

	const auto pluginInfo = SFSE::PluginVersionData::GetSingleton();
	logs::info(
		"{} version {} is loading into Starfield {}",
		std::string(pluginInfo->pluginName),
		REL::Version::unpack(pluginInfo->pluginVersion).string("."),
		a_sfse->RuntimeVersion().string(".")
	);

	std::string pluginConfig = PIMUtility::PluginConfigPath();

	// do we need papyrus?
	bool registerPapyrusFunctionsResult = false;
	if (PIMInternal::PullBoolFromIniInternal(pluginConfig, "Papyrus", "bRegisterPapyrusFunctions", false)) {
		try {
			PapyrusHook::Install();
			registerPapyrusFunctionsResult = true;
			logs::info("Papyrus functions registered");
		} catch (...) {
			logs::info("Could not register Papyrus functions");
		}
	} else logs::info("Papyrus functions disabled");

	// do we need console?
	bool registerConsoleCommandsResult = false;
	if (PIMInternal::PullBoolFromIniInternal(pluginConfig, "Console", "bRegisterConsoleCommands", false)) {
		const auto messagingInterface = SFSE::GetMessagingInterface();
		registerConsoleCommandsResult = (messagingInterface && messagingInterface->RegisterListener(MessageCallback));
		if (registerConsoleCommandsResult) logs::info("Trying to register console commands");
		else logs::info("Could not register console commands");
	} else logs::info("Console commands disabled");

	if (registerPapyrusFunctionsResult || registerConsoleCommandsResult) {
		PIMInternal::usePrettyPrint = PIMInternal::PullBoolFromIniInternal(pluginConfig, "General", "bUsePrettyPrint", false);
		logs::info("Pretty-print mode value is {}", PIMInternal::usePrettyPrint);
		PIMInternal::useTranslation = PIMInternal::PullBoolFromIniInternal(pluginConfig, "General", "bUseTranslation", false);
		logs::info("Translation mode value is {}", PIMInternal::useTranslation);
		return true;
	}

	logs::info("Both Papyrus functions and console commands are off, unload plugin");
	return false;

}
