#pragma once
#include "hpp/redata.hpp"
#include "hpp/settings.hpp"

namespace UCProcess {

	std::set<RE::TESObjectWEAP*> cutters;

	enum class ProcMode : std::uint8_t {
		ammoCapacity = 0,
		energyDamage,
		recoilMult,
		maxRange,
	};

	bool SetAmmoCapacity(RE::TESObjectWEAPInstanceData* data, std::string weaponFormID) {
		auto ammoData = reinterpret_cast<UCWeaponData::AmmoData*>(data->WeaponAmmoData);
		if (ammoData) {
			REX::INFO("Weapon {} old ammo capacity = {}", weaponFormID, ammoData->ammoCapacity);
			ammoData->ammoCapacity = UCSettings::iNewAmmoCapacity;
			return true;
		} else {
			REX::INFO("Weapon {} has no ammo data", weaponFormID);
		}
		return false;
	}

	bool SetEnergyDamage(RE::TESObjectWEAPInstanceData* data, RE::TESForm* energyDamageType, std::string weaponFormID) {
		auto damageData = data->WeaponDamage;
		if (damageData) {
			auto types = damageData->damageTypes;
			if (types) {
				std::uint32_t count = 0;
				for (auto& type : *types) if (type.first == energyDamageType) {
					REX::INFO("Weapon {} old energy damage = {}", weaponFormID, type.third.i);
					type.third.i = UCSettings::iNewEnergyDamage;
					count += 1;
				}
				if (count) {
					return true;
				} else {
					REX::INFO("Weapon {} has no energy damage", weaponFormID);
				}
			} else {
				REX::INFO("Weapon {} has no damage types", weaponFormID);
			}
		} else {
			REX::INFO("Weapon {} has no damage data", weaponFormID);
		}
		return false;
	}

	bool SetRecoilMult(RE::TESObjectWEAPInstanceData* data, std::string weaponFormID) {
		auto ApplyRecoilMult = [](RE::AimModelData* modelData) {
			auto& original = modelData->recoilPerShot;
			RE::NiPoint2 previous{ original };
			original *= UCSettings::fNewRecoilMult;
			return previous;
		};
		auto aimData = reinterpret_cast<UCWeaponData::AimData*>(data->WeaponDataAim);
		if (aimData) {
			auto modelData = aimData->aimModelData;
			if (modelData) {
				auto result = ApplyRecoilMult(modelData);
				REX::INFO("Weapon {} old recoil A = {}:{}", weaponFormID, result.x, result.y);
				return true;
			} else {
				auto modelBase = aimData->aimModelTemplate;
				if (modelBase) {
					auto result = ApplyRecoilMult(&modelBase->data);
					REX::INFO("Weapon {} old recoil B = {}:{}", weaponFormID, result.x, result.y);
					return true;
				} else {
					REX::INFO("Weapon {} has no aim model data", weaponFormID);
				}
			}
		} else {
			REX::INFO("Weapon {} has no aim data", weaponFormID);
		}
		return false;
	}

	bool SetMaxRange(RE::TESObjectWEAPInstanceData* data, std::string weaponFormID) {
		std::uint32_t count = 0;
		auto damageData = data->WeaponDamage;
		if (damageData) {
			REX::INFO("Weapon {} old max range A = {}", weaponFormID, damageData->rangeMax);
			damageData->rangeMax = UCSettings::fNewMaxRange;
			count += 1;
		} else {
			REX::INFO("Weapon {} has no damage data", weaponFormID);
		}
		auto variableRangeData = reinterpret_cast<UCWeaponData::VariableRangeData*>(data->WeapDataVariableRange);
		if (variableRangeData) {
			if (variableRangeData->useVariableRange) {
				REX::INFO("Weapon {} old max range B = {}", weaponFormID, variableRangeData->distance.valueMax);
				variableRangeData->distance.valueMax = UCSettings::fNewMaxRange;
				count += 1;
			} else {
				REX::INFO("Weapon {} doesn't use variable range", weaponFormID);
			}
		} else {
			REX::INFO("Weapon {} has no variable range data", weaponFormID);
		}
		return (count != 0);
	}

	template <ProcMode mode>
	void TweakCutterVariant(RE::TESObjectWEAP* weapon, RE::TESForm* energyDamageType = nullptr) {
		auto weaponID = std::format("{:08X}", weapon->formID);
		auto weaponData = UCUtility::GetMember<RE::BSTSmartPointer<RE::TESObjectWEAPInstanceData>>(weapon, 0x248)->get();
		if (weaponData) {
			if constexpr (mode == ProcMode::ammoCapacity) SetAmmoCapacity(weaponData, weaponID);
			if constexpr (mode == ProcMode::energyDamage) SetEnergyDamage(weaponData, energyDamageType, weaponID);
			if constexpr (mode == ProcMode::recoilMult) SetRecoilMult(weaponData, weaponID);
			if constexpr (mode == ProcMode::maxRange) SetMaxRange(weaponData, weaponID);
		} else {
			REX::INFO("Weapon {} has no data", weaponID);
		}
	}

	void FindCutters(std::string source) {
		REX::INFO("Find cutters on {}", source);
		cutters.clear();
		auto formsList = UCUtility::Split(UCSettings::sCutterForms, '|');
		for (auto formString : formsList) {
			auto formPair = UCUtility::Split(formString, ':');
			if (formPair.size() == 2) try {
				auto plugin = formPair[0];
				auto id = std::stoul(formPair[1], nullptr, 0);
				auto form = UCUtility::GetFormFromFile(plugin, id);
				if (form) {
					if (form->formType == RE::FormType::kWEAP) {
						cutters.insert(static_cast<RE::TESObjectWEAP*>(form));
						REX::INFO("Weapon {:08X} added to list", form->formID);
					} else {
						REX::INFO("Form {:08X} is not a weapon", form->formID);
					}
				} else {
					REX::INFO("Form {} not found", formPair);
				}
			} catch (...) {
				REX::INFO("Incorrect form pair >> {}", formPair);
			} else {
				REX::INFO("Incorrect form pair >> {}", formPair);
			}
		}
	}

	void TweakCutters(std::string source) {
		REX::INFO("Tweak cutters on {}", source);
		if (UCSettings::bChangeAmmoCapacity)
			for (auto cutter : cutters)
				TweakCutterVariant<ProcMode::ammoCapacity>(cutter);
		if (UCSettings::bChangeEnergyDamage) {
			auto energyDamageType = RE::TESForm::LookupByID(0x60A81);
			if (energyDamageType)
				for (auto cutter : cutters)
					TweakCutterVariant<ProcMode::energyDamage>(cutter, energyDamageType);
			else REX::INFO("Energy damage type not found");
		}
		if (UCSettings::bChangeRecoilMult)
			for (auto cutter : cutters)
				TweakCutterVariant<ProcMode::recoilMult>(cutter);
		if (UCSettings::bChangeMaxRange)
			for (auto cutter : cutters)
				TweakCutterVariant<ProcMode::maxRange>(cutter);
		cutters.clear();
	}

}