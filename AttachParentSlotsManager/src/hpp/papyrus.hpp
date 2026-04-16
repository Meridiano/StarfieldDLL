#pragma once
#include "hpp/functions.hpp"

namespace SFPapyrus {

	std::string GetVersion(std::monostate) {
		static auto version = SFSE::GetPluginVersion();
		return version.string("-");
	}

	void PapyrusTrace(std::monostate, std::string logText) {
		static std::string logName{ SFSE::GetPluginName() };
		SFInternalPapyrus::OpenUserLog(logName);
		SFInternalPapyrus::TraceUser(logName, logText);
	}

	#define ItemToArmor(I) static_cast<RE::TESObjectARMO*>(I)
	#define ItemToWeapon(I) static_cast<RE::TESObjectWEAP*>(I)

	bool HasSlot(std::monostate, RE::TESForm* object, RE::BGSKeyword* slot) {
		if (object && slot) switch (*object->formType) {
			case RE::FormType::kARMO: {
				auto armor = ItemToArmor(object);
				return SFInternalArmor::HasSlot(armor, slot);
			}   break;
			case RE::FormType::kWEAP: {
				auto weapon = ItemToWeapon(object);
				return SFInternalWeapon::HasSlot(weapon, slot);
			}   break;
		}
		return false;
	}

	std::int32_t GetSlotsCount(std::monostate, RE::TESForm* object) {
		if (object) switch (*object->formType) {
			case RE::FormType::kARMO: {
				auto armor = ItemToArmor(object);
				return SFInternalArmor::GetSlotsCount(armor);
			}   break;
			case RE::FormType::kWEAP: {
				auto weapon = ItemToWeapon(object);
				return SFInternalWeapon::GetSlotsCount(weapon);
			}   break;
		}
		return 0;
	}

	RE::BGSKeyword* GetNthSlot(std::monostate, RE::TESForm* object, std::int32_t index) {
		if (object) switch (*object->formType) {
			case RE::FormType::kARMO: {
				auto armor = ItemToArmor(object);
				return SFInternalArmor::GetNthSlot(armor, index);
			}   break;
			case RE::FormType::kWEAP: {
				auto weapon = ItemToWeapon(object);
				return SFInternalWeapon::GetNthSlot(weapon, index);
			}   break;
		}
		return nullptr;
	}

	SFUtility::VectorType GetAllSlots(std::monostate, RE::TESForm* object) {
		SFUtility::VectorType result;
		if (object) switch (*object->formType) {
			case RE::FormType::kARMO: {
				auto armor = ItemToArmor(object);
				result = SFInternalArmor::GetAllSlots(armor);
			}   break;
			case RE::FormType::kWEAP: {
				auto weapon = ItemToWeapon(object);
				result = SFInternalWeapon::GetAllSlots(weapon);
			}   break;
		}
		return result;
	}

	bool RemoveAllSlots(std::monostate, RE::TESForm* object) {
		if (object) switch (*object->formType) {
			case RE::FormType::kARMO: {
				auto armor = ItemToArmor(object);
				return SFInternalArmor::RemoveAllSlots(armor);
			}   break;
			case RE::FormType::kWEAP: {
				auto weapon = ItemToWeapon(object);
				return SFInternalWeapon::RemoveAllSlots(weapon);
			}   break;
		}
		return false;
	}

	bool RemoveSlot(std::monostate, RE::TESForm* object, RE::BGSKeyword* slot) {
		if (object && slot) switch (*object->formType) {
			case RE::FormType::kARMO: {
				auto armor = ItemToArmor(object);
				return SFInternalArmor::RemoveSlot(armor, slot);
			}   break;
			case RE::FormType::kWEAP: {
				auto weapon = ItemToWeapon(object);
				return SFInternalWeapon::RemoveSlot(weapon, slot);
			}   break;
		}
		return false;
	}

	bool RemoveNthSlot(std::monostate, RE::TESForm* object, std::int32_t index) {
		if (object) switch (*object->formType) {
			case RE::FormType::kARMO: {
				auto armor = ItemToArmor(object);
				return SFInternalArmor::RemoveNthSlot(armor, index);
			}   break;
			case RE::FormType::kWEAP: {
				auto weapon = ItemToWeapon(object);
				return SFInternalWeapon::RemoveNthSlot(weapon, index);
			}   break;
		}
		return false;
	}

	bool AddSlot(std::monostate, RE::TESForm* object, RE::BGSKeyword* slot) {
		if (object && slot) switch (*object->formType) {
			case RE::FormType::kARMO: {
				auto armor = ItemToArmor(object);
				return SFInternalArmor::AddSlot(armor, slot);
			}   break;
			case RE::FormType::kWEAP: {
				auto weapon = ItemToWeapon(object);
				return SFInternalWeapon::AddSlot(weapon, slot);
			}   break;
		}
		return false;
	}

	bool ReplaceSlot(std::monostate, RE::TESForm* object, RE::BGSKeyword* slotRemove, RE::BGSKeyword* slotAdd) {
		if (object && slotRemove && slotAdd) switch (*object->formType) {
			case RE::FormType::kARMO: {
				auto armor = ItemToArmor(object);
				return SFInternalArmor::ReplaceSlot(armor, slotRemove, slotAdd);
			}   break;
			case RE::FormType::kWEAP: {
				auto weapon = ItemToWeapon(object);
				return SFInternalWeapon::ReplaceSlot(weapon, slotRemove, slotAdd);
			}   break;
		}
		return false;
	}

	#define IterateArray(I) for (auto I : list) if (I) switch (*I->formType)

	std::int32_t BatchRemoveAllSlotsV1(std::monostate, std::vector<RE::TESForm*> list) {
		std::int32_t result = 0;
		if (list.size()) IterateArray(item) {
			case RE::FormType::kARMO: {
				auto armor = ItemToArmor(item);
				result += SFInternalArmor::RemoveAllSlots(armor);
			}   break;
			case RE::FormType::kWEAP: {
				auto weapon = ItemToWeapon(item);
				result += SFInternalWeapon::RemoveAllSlots(weapon);
			}   break;
		}
		return result;
	}

	std::int32_t BatchRemoveSlotV1(std::monostate, std::vector<RE::TESForm*> list, RE::BGSKeyword* slot) {
		std::int32_t result = 0;
		if (list.size() && slot) IterateArray(item) {
			case RE::FormType::kARMO: {
				auto armor = ItemToArmor(item);
				result += SFInternalArmor::RemoveSlot(armor, slot);
			}   break;
			case RE::FormType::kWEAP: {
				auto weapon = ItemToWeapon(item);
				result += SFInternalWeapon::RemoveSlot(weapon, slot);
			}   break;
		}
		return result;
	}

	std::int32_t BatchRemoveNthSlotV1(std::monostate, std::vector<RE::TESForm*> list, std::int32_t index) {
		std::int32_t result = 0;
		if (list.size()) IterateArray(item) {
			case RE::FormType::kARMO: {
				auto armor = ItemToArmor(item);
				result += SFInternalArmor::RemoveNthSlot(armor, index);
			}   break;
			case RE::FormType::kWEAP: {
				auto weapon = ItemToWeapon(item);
				result += SFInternalWeapon::RemoveNthSlot(weapon, index);
			}   break;
		}
		return result;
	}

	std::int32_t BatchAddSlotV1(std::monostate, std::vector<RE::TESForm*> list, RE::BGSKeyword* slot) {
		std::int32_t result = 0;
		if (list.size() && slot) IterateArray(item) {
			case RE::FormType::kARMO: {
				auto armor = ItemToArmor(item);
				result += SFInternalArmor::AddSlot(armor, slot);
			}   break;
			case RE::FormType::kWEAP: {
				auto weapon = ItemToWeapon(item);
				result += SFInternalWeapon::AddSlot(weapon, slot);
			}   break;
		}
		return result;
	}

	std::int32_t BatchReplaceSlotV1(std::monostate, std::vector<RE::TESForm*> list, RE::BGSKeyword* slotRemove, RE::BGSKeyword* slotAdd) {
		std::int32_t result = 0;
		if (list.size() && slotRemove && slotAdd) IterateArray(item) {
			case RE::FormType::kARMO: {
				auto armor = ItemToArmor(item);
				result += SFInternalArmor::ReplaceSlot(armor, slotRemove, slotAdd);
			}   break;
			case RE::FormType::kWEAP: {
				auto weapon = ItemToWeapon(item);
				result += SFInternalWeapon::ReplaceSlot(weapon, slotRemove, slotAdd);
			}   break;
		}
		return result;
	}

	std::int32_t BatchRemoveAllSlotsV2(std::monostate, RE::BGSListForm* list) {
		if (list) {
			std::monostate dummy;
			auto array = SFUtility::FormListToArray(list);
			return BatchRemoveAllSlotsV1(dummy, array);
		}
		return 0;
	}

	std::int32_t BatchRemoveSlotV2(std::monostate, RE::BGSListForm* list, RE::BGSKeyword* slot) {
		if (list && slot) {
			std::monostate dummy;
			auto array = SFUtility::FormListToArray(list);
			return BatchRemoveSlotV1(dummy, array, slot);
		}
		return 0;
	}

	std::int32_t BatchRemoveNthSlotV2(std::monostate, RE::BGSListForm* list, std::int32_t index) {
		if (list) {
			std::monostate dummy;
			auto array = SFUtility::FormListToArray(list);
			return BatchRemoveNthSlotV1(dummy, array, index);
		}
		return 0;
	}

	std::int32_t BatchAddSlotV2(std::monostate, RE::BGSListForm* list, RE::BGSKeyword* slot) {
		if (list && slot) {
			std::monostate dummy;
			auto array = SFUtility::FormListToArray(list);
			return BatchAddSlotV1(dummy, array, slot);
		}
		return 0;
	}

	std::int32_t BatchReplaceSlotV2(std::monostate, RE::BGSListForm* list, RE::BGSKeyword* slotRemove, RE::BGSKeyword* slotAdd) {
		if (list && slotRemove && slotAdd) {
			std::monostate dummy;
			auto array = SFUtility::FormListToArray(list);
			return BatchReplaceSlotV1(dummy, array, slotRemove, slotAdd);
		}
		return 0;
	}

	#undef IterateArray

	void AssignSlotsV1(std::monostate, RE::TESForm* object, SFUtility::VectorType slots) {
		if (object) switch (*object->formType) {
			case RE::FormType::kARMO: {
				auto armor = ItemToArmor(object);
				SFInternalArmor::AssignSlots(armor, slots);
			}   break;
			case RE::FormType::kWEAP: {
				auto weapon = ItemToWeapon(object);
				SFInternalWeapon::AssignSlots(weapon, slots);
			}   break;
		}
	}

	void AssignSlotsV2(std::monostate, RE::TESForm* object, RE::BGSListForm* list) {
		if (object && list) {
			std::monostate dummy;
			auto array = SFUtility::FormListToArray(list, RE::FormType::kKYWD);
			auto slots = reinterpret_cast<SFUtility::VectorType*>(&array);
			AssignSlotsV1(dummy, object, *slots);
		}
	}

	bool CopySlots(std::monostate, RE::TESForm* objectFrom, RE::TESForm* objectTo) {
		if (objectFrom && objectTo) {
			auto typeLeft = *objectFrom->formType;
			auto typeRight = *objectTo->formType;
			if (typeLeft == typeRight) switch (typeLeft) {
				case RE::FormType::kARMO: {
					auto armor = ItemToArmor(objectFrom);
					auto target = ItemToArmor(objectTo);
					SFInternalArmor::CopySlots(armor, target);
				}   return true;
				case RE::FormType::kWEAP: {
					auto weapon = ItemToWeapon(objectFrom);
					auto target = ItemToWeapon(objectTo);
					SFInternalWeapon::CopySlots(weapon, target);
				}   return true;
			}
		}
		return false;
	}

	bool SwapSlots(std::monostate, RE::TESForm* objectLeft, RE::TESForm* objectRight) {
		if (objectLeft && objectRight) {
			auto typeLeft = *objectLeft->formType;
			auto typeRight = *objectRight->formType;
			if (typeLeft == typeRight) switch (typeLeft) {
				case RE::FormType::kARMO: {
					auto left = ItemToArmor(objectLeft);
					auto right = ItemToArmor(objectRight);
					SFInternalArmor::SwapSlots(left, right);
				}   return true;
				case RE::FormType::kWEAP: {
					auto left = ItemToWeapon(objectLeft);
					auto right = ItemToWeapon(objectRight);
					SFInternalWeapon::SwapSlots(left, right);
				}   return true;
			}
		}
		return false;
	}

	#undef ItemToArmor
	#undef ItemToWeapon

	void RegisterFunctions(RE::BSScript::IVirtualMachine* vm) {
		std::string className{ SFSE::GetPluginName() };
		#define BIND_METHOD(S,F) vm->BindNativeMethod(className, S, &F, true, false)
		// basic
		BIND_METHOD("GetVersion", GetVersion);
		BIND_METHOD("PapyrusTrace", PapyrusTrace);
		// getters
		BIND_METHOD("HasSlot", HasSlot);
		BIND_METHOD("GetSlotsCount", GetSlotsCount);
		BIND_METHOD("GetNthSlot", GetNthSlot);
		BIND_METHOD("GetAllSlots", GetAllSlots);
		// changers - single
		BIND_METHOD("RemoveAllSlots", RemoveAllSlots);
		BIND_METHOD("RemoveSlot", RemoveSlot);
		BIND_METHOD("RemoveNthSlot", RemoveNthSlot);
		BIND_METHOD("AddSlot", AddSlot);
		BIND_METHOD("ReplaceSlot", ReplaceSlot);
		// changers - batch - list
		BIND_METHOD("BatchRemoveAllSlotsV1", BatchRemoveAllSlotsV1);
		BIND_METHOD("BatchRemoveSlotV1", BatchRemoveSlotV1);
		BIND_METHOD("BatchRemoveNthSlotV1", BatchRemoveNthSlotV1);
		BIND_METHOD("BatchAddSlotV1", BatchAddSlotV1);
		BIND_METHOD("BatchReplaceSlotV1", BatchReplaceSlotV1);
		// changers - batch - array
		BIND_METHOD("BatchRemoveAllSlotsV2", BatchRemoveAllSlotsV2);
		BIND_METHOD("BatchRemoveSlotV2", BatchRemoveSlotV2);
		BIND_METHOD("BatchRemoveNthSlotV2", BatchRemoveNthSlotV2);
		BIND_METHOD("BatchAddSlotV2", BatchAddSlotV2);
		BIND_METHOD("BatchReplaceSlotV2", BatchReplaceSlotV2);
		// setters
		BIND_METHOD("AssignSlotsV1", AssignSlotsV1);
		BIND_METHOD("AssignSlotsV2", AssignSlotsV2);
		BIND_METHOD("CopySlots", CopySlots);
		BIND_METHOD("SwapSlots", SwapSlots);
		#undef BIND_METHOD
	}

	struct PapyrusHook {
		static void Modified(RE::BSScript::IVirtualMachine** vmHandle) {
			Original(vmHandle);
			if (vmHandle) {
				auto vm = *vmHandle;
				if (vm) RegisterFunctions(vm);
				else REX::FAIL("Virtual machine error");
			} else REX::FAIL("Virtual machine handle error");
		}
		static inline REL::THook Original{ REL::ID(116472), 0x802, Modified };
	};

	struct ReloadHook {
		static void Modified(std::int64_t a1, std::int64_t a2) {
			REX::INFO("Reset data holder on reload");
			for (auto& entry : SFUtility::dataHolder)
				if (auto array = entry.first; array)
					array->Reset();
			SFUtility::dataHolder.clear();
			return Original(a1, a2);
		}
		static inline REL::THook Original{ REL::ID(99468), 0x12AA, Modified };
	};

}