#pragma once

#include "hpp/functions.hpp"

namespace CCRHooks {

    class EventHandler final:
        public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
        public RE::BSTEventSink<RE::TESEquipEvent> {
    public:
        static EventHandler* GetSingleton() {
            static EventHandler self;
            auto address = std::addressof(self);
            logs::info("Event handler called, address = {:X}", (std::uint64_t)address);
            return address;
        }
        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) {
            // assign
            auto sName = std::string(a_event.menuName);
            int iOpen = a_event.opening ? 1 : 0;
            // process
            if (sName == "HUDMessagesMenu" && iOpen == 1) {
                auto loadType = CCRUtility::GameLoadType();
                if (loadType > 0) {
                    saveLoaded = false;
                    CCRFunctions::RunGameCommands(0); // process 0
                    CCRFunctions::RunGameCommands(loadType); // process 1 or 2
                }
            }
            CCRFunctions::RunMenuCommands(sName, iOpen);
            return RE::BSEventNotifyControl::kContinue;
        }
        RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent& a_event, RE::BSTEventSource<RE::TESEquipEvent>* a_source) {
            auto actorStrings = CCRUtility::GetAllStrings(a_event.actor.get());
            // auto actorCheck = CCRUtility::VectorToString(actorStrings);

            auto itemStrings = CCRUtility::GetAllStrings(RE::TESForm::LookupByID(a_event.baseObject));
            // auto itemCheck = CCRUtility::VectorToString(itemStrings);

            CCRFunctions::RunEquipCommands(actorStrings, itemStrings, a_event.equipped);

            //logs::info("{}EQUIP / {} / {}", a_event.equipped ? "" : "UN", actorCheck, itemCheck);

            return RE::BSEventNotifyControl::kContinue;
        }
    };

    class LoadGameHook {
    public:
        static void Install() {
            static REL::Relocation pcVTable{ REL::ID(423292) };
            OriginalLoadGame = pcVTable.write_vfunc(0x1B, ModifiedLoadGame);
        }
    private:
        static std::int64_t ModifiedLoadGame(RE::PlayerCharacter* a1, RE::BGSLoadFormBuffer* a2) {
            saveLoaded = true;
            return OriginalLoadGame(a1, a2);
        }
        static inline REL::Relocation<decltype(ModifiedLoadGame)> OriginalLoadGame;
    };

    void InstallHooks(std::uint8_t type) {
        switch (type) {
            case 0: // kPostLoad
                LoadGameHook::Install();
                return;
            case 1: // kPostDataLoad
                if (auto handler = EventHandler::GetSingleton(); handler) {
                    if (auto sfui = RE::UI::GetSingleton(); sfui)
                        sfui->RegisterSink<RE::MenuOpenCloseEvent>(handler);
                    if (auto equip = RE::TESEquipEvent::GetEventSource(); equip)
                        equip->RegisterSink(handler);
                }
                return;
        }
    }

}