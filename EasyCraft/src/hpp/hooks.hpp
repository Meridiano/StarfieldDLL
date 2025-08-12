#pragma once

#include "hpp/process.hpp"

namespace EZCHooks {

	std::mutex hookLock;

	class COBJ_Hook {
	private:
		struct Virtual {
			static void NEW(RE::BGSConstructibleObject* cobj) {
				OLD(cobj);
				std::lock_guard<std::mutex> protect(hookLock);
				if (cobj) EZCProcess::cobjList.insert(cobj);
			}
			static inline REL::Relocation<decltype(NEW)> OLD;
		};
	public:
		static void Install() {
			REL::Relocation reloc{ REL::ID(410228) };
			Virtual::OLD = reloc.write_vfunc(0x1F, Virtual::NEW);
			REX::INFO("COBJ hook installed");
		}
	};

	class RSPJ_Hook {
	private:
		struct Virtual {
			static void NEW(RE::BGSResearchProjectForm* rspj) {
				OLD(rspj);
				std::lock_guard<std::mutex> protect(hookLock);
				if (rspj) EZCProcess::rspjList.insert(rspj);
			}
			static inline REL::Relocation<decltype(NEW)> OLD;
		};
	public:
		static void Install() {
			REL::Relocation reloc{ REL::ID(405103) };
			Virtual::OLD = reloc.write_vfunc(0x1F, Virtual::NEW);
			REX::INFO("RSPJ hook installed");
		}
	};

	class ReloadHookA {
	private:
		struct Call {
			static void NEW(std::int64_t a1, std::int64_t a2) {
				std::string info = "DataReloaded";
				EZCProcess::PurgeList(COBJ, info);
				EZCProcess::PurgeList(RSPJ, info);
				return OLD(a1, a2);
			}
			static inline REL::Relocation<decltype(NEW)> OLD;
		};
	public:
		static void Install() {
			REL::Relocation reloc{ REL::ID(99468), 0x129A };
			Call::OLD = reloc.write_call<5>(Call::NEW);
			REX::INFO("Reload hook A installed");
		}
	};

	class ReloadHookB {
	private:
		struct Call {
			static std::int64_t NEW() {
				EZCProcess::ProcessForms("DataReloaded");
				return OLD();
			}
			static inline REL::Relocation<decltype(NEW)> OLD;
		};
	public:
		static void Install() {
			REL::Relocation reloc{ REL::ID(99468), 0x1907 };
			Call::OLD = reloc.write_call<5>(Call::NEW);
			REX::INFO("Reload hook B installed");
		}
	};

	class EventHandler:
		public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
	public:
		static void Install() {
			static EventHandler self;
			auto handler = std::addressof(self);
			if (auto ui = RE::UI::GetSingleton(); ui) {
				using type = RE::BSTEventSource<RE::MenuOpenCloseEvent>;
				auto source = EZCUtility::GetMember<type>(ui, 0x20);
				source->RegisterSink(handler);
				REX::INFO("Interface sink installed");
			}
		}
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) {
			if (EZCProcess::processMenu && a_event.menuName == "HUDMessagesMenu" && a_event.opening) EZCProcess::ProcessForms("Interface");
			return RE::BSEventNotifyControl::kContinue;
		}
	};

}