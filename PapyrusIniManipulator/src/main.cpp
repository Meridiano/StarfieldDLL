#include "hpp/papyrus.hpp"
#include "hpp/console.hpp"

void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
	if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
		std::string message = PIMConsole::Register() ? "installed" : "not installed";
		REX::INFO("Console commands {}", message);
	} else return;
}

class PapyrusHook {
private:
	struct PapyrusCall {
		static void NEW(RE::BSScript::IVirtualMachine** a_vm) {
			// call original
			OLD(a_vm);
			// register new
			PIMPapyrus::RegisterFunctions(*a_vm);
		}
		static inline REL::Relocation<decltype(NEW)> OLD;
	};
public:
	static void Install() {
		REL::Relocation target{ REL::ID(116472), 0x802 };
		if (GetU8(target.get()) != 0xE8) throw std::exception("incorrect opcode");
		PapyrusCall::OLD = target.write_call<5>(PapyrusCall::NEW);
	}
};

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
	SFSE::InitInfo info = {
		.logPattern = "%d.%m.%Y %H:%M:%S [%s:%#] %v",
		.trampoline = true,
		.trampolineSize = 64
	};
	SFSE::Init(a_sfse, info);

	const auto gameInfo = a_sfse->RuntimeVersion().string(".");
	REX::INFO("Starfield v{}", gameInfo);

	std::string pluginConfig = PIMUtility::PluginConfigPath();

	// do we need papyrus?
	bool registerPapyrusFunctionsResult = false;
	if (PIMInternal::PullBoolFromIniInternal(pluginConfig, "Papyrus", "bRegisterPapyrusFunctions", false)) {
		try {
			PapyrusHook::Install();
			registerPapyrusFunctionsResult = true;
			REX::INFO("Papyrus functions registered");
		} catch (...) {
			REX::INFO("Could not register Papyrus functions");
		}
	} else REX::INFO("Papyrus functions disabled");

	// do we need console?
	bool registerConsoleCommandsResult = false;
	if (PIMInternal::PullBoolFromIniInternal(pluginConfig, "Console", "bRegisterConsoleCommands", false)) {
		const auto messagingInterface = SFSE::GetMessagingInterface();
		registerConsoleCommandsResult = (messagingInterface && messagingInterface->RegisterListener(MessageCallback));
		if (registerConsoleCommandsResult) REX::INFO("Trying to register console commands");
		else REX::INFO("Could not register console commands");
	} else REX::INFO("Console commands disabled");

	if (registerPapyrusFunctionsResult || registerConsoleCommandsResult) {
		PIMInternal::usePrettyPrint = PIMInternal::PullBoolFromIniInternal(pluginConfig, "General", "bUsePrettyPrint", false);
		REX::INFO("Pretty-print mode value is {}", PIMInternal::usePrettyPrint);
		PIMInternal::useTranslation = PIMInternal::PullBoolFromIniInternal(pluginConfig, "General", "bUseTranslation", false);
		REX::INFO("Translation mode value is {}", PIMInternal::useTranslation);
		return true;
	}

	REX::INFO("Both Papyrus functions and console commands are off, unload plugin");
	return false;

}
