#pragma once
#include "hpp/settings.hpp"

namespace SlowTimeProcess {

	bool bSlowTimeActive = false;
	SlowTimeUtility::WaveAudioFile soundPos;
	SlowTimeUtility::WaveAudioFile soundNeg;
	std::optional<sf::Sound> soundHandle;

	class Blacklist {
	private:
		std::vector<std::string> list;
		decltype(list)::iterator bgn;
		decltype(list)::iterator end;
	public:
		Blacklist(std::string source) {
			list = SlowTimeUtility::Split(source, '|');
			bgn = list.begin();
			end = list.end();
		}
		bool IsOpen(RE::UI* sfui) {
			for (std::string menu : list) if (sfui->IsMenuOpen(menu)) return true;
			return false;
		}
		bool Contains(std::string menu) {
			return (std::find(bgn, end, menu) != end);
		}
	};

	void ChangeMouseSpeed(std::uint8_t mode, float mult = 1.0F) {
		if (auto ini = RE::INIPrefSettingCollection::GetSingleton(); ini) {
			auto fMouse = ini->GetSetting("fMouseHeadingSensitivity:Controls");
			auto fGamepad = ini->GetSetting("fGamepadHeadingSensitivity:Controls");
			if (fMouse && fGamepad) {
				static float fMouseBackup = 0.0F;
				static float fGamepadBackup = 0.0F;
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

	void SetSlowTime(std::uint8_t mode, bool sound, bool message) {
		if (mode == 2) {
			static Blacklist blHard(SlowTimeSettings::sBlacklistHard);
			static Blacklist blSoft(SlowTimeSettings::sBlacklistSoft);
			static auto sfui = RE::UI::GetSingleton();
			if (!sfui) return;
			if (blHard.IsOpen(sfui)) return;
			if (blSoft.IsOpen(sfui)) return;
			if (!sfui->IsMenuOpen("HUDMessagesMenu")) return;
		}
		bool newState = [](std::uint8_t value) {
			if (value == 0) return false;
			if (value == 1) return true;
			return !bSlowTimeActive;
		}(mode);
		float global = (newState ? SlowTimeSettings::fGlobalMult : 1.0F);
		float player = (newState ? SlowTimeSettings::fPlayerMult : 1.0F);
		float mouse = SlowTimeSettings::fMouseMult / SlowTimeSettings::fPlayerMult;
		SlowTimeUtility::GameTimeMultiplier(global, player).Apply();
		ChangeMouseSpeed(newState ? 1 : 2, mouse);
		bSlowTimeActive = newState;
		if (sound) {
			auto& wave = (newState ? soundPos : soundNeg);
			if (std::uint8_t error = wave.Play(soundHandle); error) REX::INFO("Error on sound {} = {}", wave.Filename(), error);
		}
		if (message) {
			auto& text = (newState ? SlowTimeSettings::sMessageOn : SlowTimeSettings::sMessageOff);
			if (text.size() > 0) SlowTimeUtility::DebugNotification(text);
		}
		if (SlowTimeSettings::fImageSpacePower > 0.0F) {
			auto imageSpace = SlowTimeUtility::ImageSpaceHelper(SlowTimeSettings::pImageSpace.first, SlowTimeSettings::pImageSpace.second, SlowTimeSettings::fImageSpacePower);
			if (std::uint8_t error = newState ? imageSpace.Apply() : imageSpace.Remove(); error) REX::INFO("ImageSpace error on {} = {}", newState ? "apply" : "remove", error);
		}
	}

	void ProcessActorValue(RE::Main* main, float factor) {
		bool bData = SlowTimeUtility::IsDataLoaded();
		bool bGameNotPaused = !main->isGameMenuPaused;
		if (bData && bGameNotPaused && bSlowTimeActive) {
			auto pc = RE::PlayerCharacter::GetSingleton();
			auto avSource = SlowTimeUtility::ActorValueHelper::Locate(SlowTimeSettings::pEditorIDs.first);
			auto avTarget = SlowTimeUtility::ActorValueHelper::Locate(SlowTimeSettings::pEditorIDs.second);
			if (pc && avSource && avTarget) {
				auto check = SlowTimeSettings::pThresholdValue;
				if (SlowTimeUtility::ActorValueHelper::Check(pc, avSource, avTarget, check.first, SlowTimeSettings::bInvertThreshold, check.second)) {
					if (avSource->flags & 0x88) return;
					auto damage = SlowTimeSettings::pDamageValue;
					SlowTimeUtility::ActorValueHelper::Modify(pc, avSource, avTarget, damage.first, damage.second / factor);
				} else SetSlowTime(0, true, true);
			}
		}
	}

	void HotkeyThread(std::uint32_t sleepTime) {
		bool keyListener = true;
		auto main = RE::Main::GetSingleton();
		if (main) while (!main->quitGame) {
			if (SlowTimeUtility::GameIsFocused()) {
				bool keyPressed = SlowTimeUtility::HotkeyPressed(SlowTimeSettings::bGamepadMode, SlowTimeSettings::iHotkey);
				if (keyPressed) {
					if (keyListener) {
						keyListener = false;
						bool toggle = SlowTimeUtility::ModifierPressed(SlowTimeSettings::bGamepadMode, SlowTimeSettings::iModifier);
						if (toggle) SetSlowTime(2, true, true);
					}
				} else keyListener = true;
			}
			if (SlowTimeSettings::bEnableAV) ProcessActorValue(main, 1000.0F / sleepTime);
			Sleep(sleepTime);
		}
		soundHandle.reset();
		REX::INFO("Sound handle erased");
	}

	class EventHandler : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
	public:
		static EventHandler* GetSingleton() {
			static EventHandler self;
			auto address = std::addressof(self);
			REX::INFO("Event handler called, address = {:X}", (std::uint64_t)address);
			return address;
		}
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) {
			if (bSlowTimeActive) {
				std::string menu = a_event.menuName.data();
				static Blacklist blacklist(SlowTimeSettings::sBlacklistHard);
				if (blacklist.Contains(menu) && a_event.opening) SetSlowTime(0, true, true);
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
		if (a_msg->type == SFSE::MessagingInterface::kPostLoad) {
			// load sounds
			auto prefix = PluginPrefix;
			soundPos = SlowTimeUtility::WaveAudioFile(prefix + ".On.wav", SlowTimeSettings::fSoundVolume);
			soundNeg = SlowTimeUtility::WaveAudioFile(prefix + ".Off.wav", SlowTimeSettings::fSoundVolume);
		} else if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
			// register for key
			std::thread(HotkeyThread, 50).detach();
			// register for menu
			auto sfui = RE::UI::GetSingleton();
			auto handler = EventHandler::GetSingleton();
			if (sfui && handler) {
				SlowTimeUtility::GetMenuEventSource(sfui)->RegisterSink(handler);
				REX::INFO("Menu listener registered");
			} else REX::FAIL("Menu listener not registered");
			// save mouse values
			ChangeMouseSpeed(0);
		}
	}

}