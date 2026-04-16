#pragma once
#include "hpp/utility.hpp"

namespace SFInternalPapyrus {

	bool OpenUserLog(std::string logName) {
		using typeM = void*;
		using typeF = bool(typeM, RE::BSFixedString*);
		static REL::Relocation<typeM*> manager{ REL::ID(937585) };
		static REL::Relocation<typeF*> function{ REL::ID(116493) };
		RE::BSFixedString fixedName{ logName };
		return function(*manager, &fixedName);
	}

	bool TraceUser(std::string logName, std::string logText) {
		using typeF = bool(std::int64_t, std::int64_t, std::int64_t, RE::BSFixedString*, RE::BSFixedString*, std::int32_t);
		static REL::Relocation<typeF*> function{ REL::ID(117324) };
		RE::BSFixedString fixedName{ logName };
		RE::BSFixedString fixedText{ logText };
		return function(NULL, NULL, NULL, &fixedName, &fixedText, NULL);
	}

}

#define GetArrayPointer(F,O) SFUtility::GetMember<SFUtility::AttachParentArray>(F, O)

namespace SFInternalArmor {

	#define GetArrayPointerARMO(F) GetArrayPointer(F,0x320)

	bool HasSlot(RE::TESObjectARMO* armor, RE::BGSKeyword* slot) {
		auto array = GetArrayPointerARMO(armor);
		return array->HasKeyword(slot);
	}

	std::int32_t GetSlotsCount(RE::TESObjectARMO* armor) {
		auto array = GetArrayPointerARMO(armor);
		return array->GetKeywordsCount<std::int32_t>();
	}

	RE::BGSKeyword* GetNthSlot(RE::TESObjectARMO* armor, std::int32_t index) {
		auto array = GetArrayPointerARMO(armor);
		return array->GetNthKeyword(index);
	}

	std::vector<RE::BGSKeyword*> GetAllSlots(RE::TESObjectARMO* armor) {
		auto array = GetArrayPointerARMO(armor);
		return array->GetAllKeywords();
	}

	bool RemoveAllSlots(RE::TESObjectARMO* armor) {
		auto array = GetArrayPointerARMO(armor);
		return array->RemoveAllKeywords();
	}

	bool RemoveSlot(RE::TESObjectARMO* armor, RE::BGSKeyword* slot) {
		auto array = GetArrayPointerARMO(armor);
		return array->RemoveKeyword(slot);
	}

	bool RemoveNthSlot(RE::TESObjectARMO* armor, std::int32_t index) {
		auto array = GetArrayPointerARMO(armor);
		return array->RemoveNthKeyword(index);
	}

	bool AddSlot(RE::TESObjectARMO* armor, RE::BGSKeyword* slot) {
		auto array = GetArrayPointerARMO(armor);
		return array->AddKeyword(slot);
	}

	bool ReplaceSlot(RE::TESObjectARMO* armor, RE::BGSKeyword* slotRemove, RE::BGSKeyword* slotAdd) {
		auto array = GetArrayPointerARMO(armor);
		return array->ReplaceKeyword(slotRemove, slotAdd);
	}

	void AssignSlots(RE::TESObjectARMO* armor, std::vector<RE::BGSKeyword*> slots) {
		auto array = GetArrayPointerARMO(armor);
		array->AssignKeywords(slots);
	}

	void CopySlots(RE::TESObjectARMO* armor, RE::TESObjectARMO* target) {
		auto array = GetAllSlots(armor);
		AssignSlots(target, array);
	}

	void SwapSlots(RE::TESObjectARMO* left, RE::TESObjectARMO* right) {
		auto arrayLeft = GetAllSlots(left);
		auto arrayRight = GetAllSlots(right);
		AssignSlots(left, arrayRight);
		AssignSlots(right, arrayLeft);
	}

	#undef GetArrayPointerARMO

}

namespace SFInternalWeapon {

	#define GetArrayPointerWEAP(F) GetArrayPointer(F,0x250)

	bool HasSlot(RE::TESObjectWEAP* weapon, RE::BGSKeyword* slot) {
		auto array = GetArrayPointerWEAP(weapon);
		return array->HasKeyword(slot);
	}

	std::int32_t GetSlotsCount(RE::TESObjectWEAP* weapon) {
		auto array = GetArrayPointerWEAP(weapon);
		return array->GetKeywordsCount<std::int32_t>();
	}

	RE::BGSKeyword* GetNthSlot(RE::TESObjectWEAP* weapon, std::int32_t index) {
		auto array = GetArrayPointerWEAP(weapon);
		return array->GetNthKeyword<std::int32_t>(index);
	}

	std::vector<RE::BGSKeyword*> GetAllSlots(RE::TESObjectWEAP* weapon) {
		auto array = GetArrayPointerWEAP(weapon);
		return array->GetAllKeywords();
	}

	bool RemoveAllSlots(RE::TESObjectWEAP* weapon) {
		auto array = GetArrayPointerWEAP(weapon);
		return array->RemoveAllKeywords();
	}

	bool RemoveSlot(RE::TESObjectWEAP* weapon, RE::BGSKeyword* slot) {
		auto array = GetArrayPointerWEAP(weapon);
		return array->RemoveKeyword(slot);
	}

	bool RemoveNthSlot(RE::TESObjectWEAP* weapon, std::int32_t index) {
		auto array = GetArrayPointerWEAP(weapon);
		return array->RemoveNthKeyword(index);
	}

	bool AddSlot(RE::TESObjectWEAP* weapon, RE::BGSKeyword* slot) {
		auto array = GetArrayPointerWEAP(weapon);
		return array->AddKeyword(slot);
	}

	bool ReplaceSlot(RE::TESObjectWEAP* weapon, RE::BGSKeyword* slotRemove, RE::BGSKeyword* slotAdd) {
		auto array = GetArrayPointerWEAP(weapon);
		return array->ReplaceKeyword(slotRemove, slotAdd);
	}

	void AssignSlots(RE::TESObjectWEAP* weapon, std::vector<RE::BGSKeyword*> slots) {
		auto array = GetArrayPointerWEAP(weapon);
		array->AssignKeywords(slots);
	}

	void CopySlots(RE::TESObjectWEAP* weapon, RE::TESObjectWEAP* target) {
		auto array = GetAllSlots(weapon);
		AssignSlots(target, array);
	}

	void SwapSlots(RE::TESObjectWEAP* left, RE::TESObjectWEAP* right) {
		auto arrayLeft = GetAllSlots(left);
		auto arrayRight = GetAllSlots(right);
		AssignSlots(left, arrayRight);
		AssignSlots(right, arrayLeft);
	}

	#undef GetArrayPointerWEAP

}

#undef GetArrayPointer