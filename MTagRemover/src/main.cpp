bool RemoveMTag() {
	REL::Relocation<const char*> target{ REL::ID(460978) };
	std::size_t count = 4;
	if (std::strncmp(target.get(), "[M] ", count)) return false;
	REL::WriteSafeFill(target.address(), 0, count);
	return true;
}

void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
	if (a_msg->type == SFSE::MessagingInterface::kPostLoad) {
		REX::INFO("M-Tag patch {}", RemoveMTag() ? "applied" : "failed");
	}
}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
	SFSE::Init(a_sfse, true);
	const auto msgInterface = SFSE::GetMessagingInterface();
	if (msgInterface && msgInterface->RegisterListener(MessageCallback)) {
		REX::INFO("Message listener registered");
	} else {
		REX::FAIL("Message listener not registered");
	}
	return true;
}
