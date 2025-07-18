#pragma once
#include "hpp/settings.hpp"

namespace SlowTimeProcess {

	bool bSlowTimeActive = false;
	float fMouseBackup = 0.0F;
	float fGamepadBackup = 0.0F;

	class Blacklist {
	private:
		std::vector<std::string> list;
	public:
		Blacklist() {
			list = SlowTimeUtility::Split(SlowTimeSettings::sBlacklist, '|');
		}
		bool IsOpen(RE::UI* sfui) {
			for (std::string menu : list) if (sfui->IsMenuOpen(menu)) return true;
			return false;
		}
		bool Contains(std::string menu) {
			auto bgn = list.begin();
			auto end = list.end();
			return (std::find(bgn, end, menu) != end);
		}
	};

	void ChangeMouseSpeed(int mode, float mult = 1.0F) {
		if (auto ini = RE::INIPrefSettingCollection::GetSingleton(); ini) {
			auto fMouse = ini->GetSetting("fMouseHeadingSensitivity:Controls");
			auto fGamepad = ini->GetSetting("fGamepadHeadingSensitivity:Controls");
			if (fMouse && fGamepad) {
				if (mode == 2) {
					// restore backup
					fMouse->SetFloat(fMouseBackup);
					fGamepad->SetFloat(fGamepadBackup);
				} else {
					// save backup
					fMouseBackup = fMouse->GetFloat();
					fGamepadBackup = fGamepad->GetFloat();
					if (mode == 1) {
						// apply changes
						fMouse->SetFloat(fMouseBackup * mult);
						fGamepad->SetFloat(fGamepadBackup * mult);
					}
				}
			}
		}
	}
	
	void SetSlowTime(bool toggle) {
		float global = (toggle ? SlowTimeSettings::fGlobalMult : 1.0F);
		float player = (toggle ? SlowTimeSettings::fPlayerMult : 1.0F);
		float mouse = SlowTimeSettings::fMouseMult / SlowTimeSettings::fPlayerMult;
		SlowTimeUtility::GameTimeMultiplier(global, player).Apply();
		ChangeMouseSpeed(toggle ? 1 : 2, mouse);
		std::string sound = std::format("Data\\SFSE\\Plugins\\SlowTimeSFSE.{}.wav", toggle ? "On" : "Off");
		SlowTimeUtility::PlayWAV(sound);
	}

	void ToggleSlowTime() {
		if (auto sfui = RE::UI::GetSingleton(); sfui) {
			if (Blacklist().IsOpen(sfui)) return;
			if (sfui->IsMenuOpen("HUDMessagesMenu")) {
				bSlowTimeActive = !bSlowTimeActive;
				SetSlowTime(bSlowTimeActive);
				auto message = (bSlowTimeActive ? SlowTimeSettings::sMessageOn : SlowTimeSettings::sMessageOff);
				if (message.size() > 0) SlowTimeUtility::DebugNotification(message);
			}
		}
	}
	
	void ProcessActorValue(RE::Main* main, float factor) {
		bool bData = SlowTimeUtility::IsDataLoaded();
		bool bMain = main && !main->isGameMenuPaused;
		if (bData && bMain && bSlowTimeActive) {
			auto pc = RE::PlayerCharacter::GetSingleton();
			auto av = SlowTimeUtility::LocateActorValue(SlowTimeSettings::sEditorID);
			if (pc && av) {
				auto check = SlowTimeSettings::pLowValue;
				if (SlowTimeUtility::CheckActorValue(pc, av, check.first, check.second)) {
					auto damage = SlowTimeSettings::pDamageValue;
					SlowTimeUtility::DamageActorValue(pc, av, damage.first, damage.second / factor);
				} else {
					SlowTimeProcess::bSlowTimeActive = false;
					SlowTimeProcess::SetSlowTime(false);
				}
			}
		}
	}

	void HotkeyThread(std::uint32_t sleepTime) {
		bool keyListener = true;
		auto main = RE::Main::GetSingleton();
		while (true) {
			bool keyPressed = SlowTimeUtility::HotkeyPressed(SlowTimeSettings::bGamepadMode, SlowTimeSettings::iHotkey);
			if (keyPressed) {
				if (keyListener) {
					keyListener = false;
					bool toggle = SlowTimeUtility::ModifierPressed(SlowTimeSettings::bGamepadMode, SlowTimeSettings::iModifier);
					if (toggle) ToggleSlowTime();
				}
			} else keyListener = true;
			if (SlowTimeSettings::bEnableAV) ProcessActorValue(main, 1000.0F / sleepTime);
			Sleep(sleepTime);
		}
	}

	class EventHandler:
		public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
	public:
		static EventHandler* GetSingleton() {
			static EventHandler self;
			auto address = std::addressof(self);
			REX::INFO("Event handler called, address = {:X}", (std::uint64_t)address);
			return address;
		}
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) {
			if (auto menu = std::string(a_event.menuName); SlowTimeProcess::bSlowTimeActive && Blacklist().Contains(menu) && a_event.opening) {
				SlowTimeProcess::bSlowTimeActive = false;
				SlowTimeProcess::SetSlowTime(false);
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
		if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
			// register for key
			std::thread(HotkeyThread, 50).detach();
			// register for menu
			auto sfui = RE::UI::GetSingleton();
			auto handler = EventHandler::GetSingleton();
			if (sfui && handler) {
				SlowTimeUtility::GetMenuEventSource(sfui)->RegisterSink(handler);
				REX::INFO("Menu listener registered");
			} else REX::INFO("Menu listener not registered");
			// save mouse values
			ChangeMouseSpeed(0);
		} else return;
	}

}