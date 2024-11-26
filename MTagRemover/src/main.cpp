bool RemoveMTag() {
	REL::Relocation<const char*> target{ REL::ID(410406) };
	std::size_t count = 4;
	if (strncmp(target.get(), "[M] ", count)) return false;
	REL::safe_fill(target.address(), 0, count);
	return true;
}

void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
	if (a_msg->type == SFSE::MessagingInterface::kPostLoad) {
		logs::info("Plugin loaded, M-Tag patch {}", RemoveMTag() ? "done" : "failed");
	}
}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
	SFSE::Init(a_sfse, true);
	const auto msgInterface = SFSE::GetMessagingInterface();
	if (msgInterface && msgInterface->RegisterListener(MessageCallback)) {
		logs::info("Message listener registered");
	} else {
		SFSE::stl::report_and_fail("Message listener not registered");
	}
	return true;
}
