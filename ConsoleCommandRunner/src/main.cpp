#include "hpp/hooks.hpp"

void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
    if (a_msg->type == SFSE::MessagingInterface::kPostLoad) {
        CCRFunctions::StoreCommands();
        CCRHooks::InstallHooks();
    } else if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
        CCRFunctions::RunDataCommands();
        auto sfui = RE::UI::GetSingleton();
        auto handler = CCRHooks::EventHandler::GetSingleton();
        if (sfui && handler) sfui->RegisterSink(handler);
    } else return;
}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
    SFSE::Init(a_sfse, false);
    SFSE::AllocTrampoline(64);

    logs::init();
    spdlog::set_pattern("%d.%m.%Y %H:%M:%S [%s:%#] %v");

    const auto pluginInfo = SFSE::PluginVersionData::GetSingleton();
    logs::info(
        "{} version {} is loading into Starfield {}",
        std::string(pluginInfo->pluginName),
        REL::Version::unpack(pluginInfo->pluginVersion).string("."),
        a_sfse->RuntimeVersion().string(".")
    );

    const auto messagingInterface = SFSE::GetMessagingInterface();
    if (messagingInterface && messagingInterface->RegisterListener(MessageCallback)) {
        logs::info("Message listener registered");
    } else {
        SFSE::stl::report_and_fail("Message listener not registered");
    }
    return true;
}
