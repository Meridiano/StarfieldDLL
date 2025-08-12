#pragma once

#include "hpp/settings.hpp"

namespace EZCProcess {

	std::set<RE::TESForm*> exceptions;
	std::set<RE::BGSConstructibleObject*> cobjList;
	std::set<RE::BGSResearchProjectForm*> rspjList;
	RE::TESForm* credits = nullptr;
	bool processMenu = false;
	
	void SetupExceptions(RE::TESDataHandler* tesDH) {
		exceptions.clear();
		if (!tesDH) return;
		fs::path dirPath = "Data/SFSE/Plugins";
		if (fs::exists(dirPath)) {
			std::string type = ".ini";
			for (fs::directory_entry fileEntry : fs::directory_iterator(dirPath)) {
				fs::path filePath = fileEntry.path();
				if (fileEntry.is_regular_file() && filePath.extension() == type) {
					auto fileName = filePath.filename().string();
					if (fileName.length() > 24 && strnicmp(fileName.data(), "EasyCraft.Exception.", 20) == 0) {
						REX::INFO("Reading exception file >> {}", fileName);
						// read ini
						mINI::INIFile file(filePath);
						if (mINI::INIStructure ini; file.read(ini)) {
							for (auto sectionIterator : ini) {
								auto section = sectionIterator.first;
								for (auto keyIterator : sectionIterator.second) {
									auto key = keyIterator.first;
									auto value = keyIterator.second;
									bool badValue = true;
									if (auto temp = EZCUtility::ConvertTo<std::uint32_t>(value); temp.first) {
										std::uint32_t valueUInt32 = temp.second;
										auto form = EZCUtility::GetFormFromFile(section, valueUInt32);
										if (form && exceptions.count(form) == 0) {
											exceptions.insert(form);
											auto type = EZCUtility::CustomFormType(form);
											REX::INFO("Exception added >> {}.{:X}", type, form->formID);
											badValue = false;
										}
									}
									if (badValue) REX::INFO("Bad value >> {}|{}|{}", section, key, value);
								}
							}
						} else REX::INFO("Bad ini-file structure >> {}", fileName);
					}
				}
			}
		}
	}

	bool SetOneCredit(RE::BGSConstructibleObject* form) {
		ComponentList* compList = form->components;
		if (compList && compList->size() > 0) {
			compList->clear();
			RE::BGSTypedFormValuePair::SharedVal count{ 1 };
			Component oneCredit{ credits, nullptr, count };
			compList->push_back(oneCredit);
			return true;
		}
		return false;
	}

	void SetCompleted(RE::BGSResearchProjectForm* form) {
		using type = void(*)(std::int64_t, std::int64_t, decltype(form));
		static REL::Relocation<type> function{ REL::ID(114789) };
		function(NULL, NULL, form);
	}

	std::uint32_t cobjCount = 0;
	std::uint32_t rspjCount = 0;
	std::uint32_t cobjCountExcept = 0;
	std::uint32_t rspjCountExcept = 0;

	bool ProcessSingleCOBJ(RE::BGSConstructibleObject* cobj) {
		if (exceptions.contains(cobj)) {
			cobjCountExcept += 1;
			return false;
		}
		return SetOneCredit(cobj);
	}

	bool ProcessSingleRSPJ(RE::BGSResearchProjectForm* rspj) {
		if (exceptions.contains(rspj)) {
			rspjCountExcept += 1;
			return false;
		}
		SetCompleted(rspj);
		return true;
	}

	void PurgeList(std::uint8_t type, std::string info) {
		switch (type) {
			case COBJ:
				REX::INFO("PurgeList:COBJ:{}", info);
				cobjCount = cobjCountExcept = 0;
				cobjList.clear();
				break;
			case RSPJ:
				REX::INFO("PurgeList:RSPJ:{}", info);
				rspjCount = rspjCountExcept = 0;
				rspjList.clear();
				break;
		}
		if (cobjList.size() + rspjList.size() == 0) {
			REX::INFO("Purge credits form");
			credits = nullptr;
		}
	}

	void ProcessForms(std::string info) {
		REX::INFO("ProcessForms:{}", info);
		if (info == "Interface") {
			processMenu = false;
			for (auto rspj : rspjList) if (rspj) rspjCount += ProcessSingleRSPJ(rspj);
			REX::INFO("RSPJ / Total {} affected / With {} exception(s)", rspjCount, rspjCountExcept);
			PurgeList(RSPJ, info);
		} else {
			auto tesDH = RE::TESDataHandler::GetSingleton();
			SetupExceptions(tesDH);
			processMenu = true;
			if (tesDH) {
				if (credits = EZCUtility::GetFormFromFile(EZCSettings::sCreditsPlugin, EZCSettings::iCreditsID); credits) {
					for (auto cobj : cobjList) if (cobj) cobjCount += ProcessSingleCOBJ(cobj);
					REX::INFO("COBJ / Total {} affected / With {} exception(s)", cobjCount, cobjCountExcept);
				} else REX::INFO("Credits form not found");
			} else REX::INFO("TESDataHandler not found");
			PurgeList(COBJ, info);
		}
	}

}