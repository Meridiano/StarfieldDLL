class EssenceHook {
private:
	struct EssenceCall {
		static std::int64_t NEW(std::int64_t arg) {
			auto out = OLD(arg);
			REX::INFO("EssenceCall:{:X}:{:X}", arg, out);
			return 0;
		}
		static inline REL::Relocation<decltype(NEW)> OLD;
	};
public:
	static void Install() {
		REL::Relocation target{ REL::ID(84490), 0x5B };
		if (GetByte(target.get()) != 0xE8) throw std::exception("incorrect opcode");
		EssenceCall::OLD = target.write_call<5>(EssenceCall::NEW);
	}
};

void MessageListener(SFSE::MessagingInterface::Message* a_msg) noexcept {
	if (a_msg->type == SFSE::MessagingInterface::kPostLoad) {
		try {
			EssenceHook::Install();
			REX::INFO("Essence hook installed");
		} catch (...) {
			REX::FAIL("Failed to install essence hook");
		}
	} else return;
}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
	SFSE::InitInfo info = {
		.logPattern = "%d.%m.%Y %H:%M:%S [%s:%#] %v",
		.trampoline = true,
		.trampolineSize = 32
	};
	SFSE::Init(a_sfse, info);

	const auto gameInfo = a_sfse->RuntimeVersion().string(".");
	REX::INFO("Starfield v{}", gameInfo);

	const auto messagingInterface = SFSE::GetMessagingInterface();
	return (messagingInterface ? messagingInterface->RegisterListener(MessageListener) : false);
}
