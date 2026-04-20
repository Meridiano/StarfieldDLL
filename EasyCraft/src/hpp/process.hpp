#pragma once

#include "hpp/settings.hpp"

namespace EZCProcess {

	std::set<RE::TESForm*> exceptions;
	std::set<RE::BGSConstructibleObject*> cobjList;
	std::set<RE::BGSResearchProjectForm*> rspjList;
	std::set<RE::BGSLegendaryItem*> lgdiList;
	RE::TESForm* credits = nullptr;
	std::stop_source stopSource;

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

	#define ProcessComponentList(CL,SF) if (CL && CL->size() > 0) { CL->resize(1); if (auto data = CL->data(); data) { data->first = credits; data->second = nullptr; data->third = count; SF; } }

	bool SetOneCredit(RE::BGSConstructibleObject* form) {
		static RE::BGSTypedFormValuePair::SharedVal count{ 1 };
		ComponentList* compList = form->components;
		if (compList && compList->size() > 0) ProcessComponentList(compList, return true);
		return false;
	}

	void SetCompleted(RE::BGSResearchProjectForm* form) {
		using type = void(*)(std::int64_t, std::int64_t, decltype(form));
		static REL::Relocation<type> function{ REL::ID(114789) };
		function(NULL, NULL, form);
	}

	bool SetOneCredit(RE::BGSLegendaryItem* form) {
		static RE::BGSTypedFormValuePair::SharedVal count{ 1 };
		auto craftStruct = EZCUtility::GetMember<EZCData::LegendaryCraftStruct>(form, 0x138);
		bool result = false;
		for (auto& rank : craftStruct->legendaryRanks) {
			auto rollList = rank.GetLegendaryRollsResources();
			ProcessComponentList(rollList, result = true);
			auto pickList = rank.GetLegendaryPicksResources();
			ProcessComponentList(pickList, result = true);
		}
		for (auto& rank : craftStruct->qualityRanks) {
			auto compList = rank.GetQualityUpgradeResources();
			ProcessComponentList(compList, result = true);
		}
		return result;
	}

	#undef ProcessComponentList

	std::uint32_t cobjCount = 0;
	std::uint32_t rspjCount = 0;
	std::uint32_t lgdiCount = 0;
	std::uint32_t cobjCountExcept = 0;
	std::uint32_t rspjCountExcept = 0;
	std::uint32_t lgdiCountExcept = 0;

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

	bool ProcessSingleLGDI(RE::BGSLegendaryItem* lgdi) {
		if (exceptions.contains(lgdi)) {
			lgdiCountExcept += 1;
			return false;
		}
		return SetOneCredit(lgdi);
	}

	void PurgeList(std::uint8_t type) {
		switch (type) {
			case COBJ:
				REX::INFO("PurgeList:COBJ");
				cobjList.clear();
				break;
			case RSPJ:
				REX::INFO("PurgeList:RSPJ");
				rspjList.clear();
				break;
			case LGDI:
				REX::INFO("PurgeList:LGDI");
				lgdiList.clear();
				break;
		}
		if (cobjList.size() + rspjList.size() + lgdiList.size() == 0) {
			REX::INFO("Purge credits form");
			credits = nullptr;
		}
	}

	void ProcessForms(std::uint8_t mode) {
		std::string info = EZCUtility::CustomFormType(mode);
		REX::INFO("ProcessForms:{}", info);
		if (mode == COBJ) {
			cobjCount = cobjCountExcept = 0;
			for (auto cobj : cobjList) if (cobj) cobjCount += ProcessSingleCOBJ(cobj);
			REX::INFO("COBJ / Total {} affected / With {} exception(s)", cobjCount, cobjCountExcept);
		}
		if (mode == RSPJ) {
			stopSource.request_stop();
			stopSource = std::stop_source();
			auto timestamp = EZCUtility::GetGameMinutesPassed();
			std::thread([](std::stop_token stopToken, float oldTime) {
				auto delay = std::format("RSPJ / Delay {}", ThreadID);
				REX::INFO("{} >> Launched", delay);
				while (true) {
					SleepFor(250);
					if (stopToken.stop_requested()) {
						REX::INFO("{} >> Cancelled", delay);
						return;
					} else {
						auto newTime = EZCUtility::GetGameMinutesPassed();
						auto timeDelta = newTime - oldTime;
						if (newTime < 0.0F) {
							REX::INFO("{} >> Interrupted", delay);
							return;
						} else if (timeDelta > EZCSettings::fResearchDelay) {
							REX::INFO("{} >> Completed", delay);
							break;
						}
					}
				}
				rspjCount = rspjCountExcept = 0;
				for (auto rspj : rspjList) if (rspj) rspjCount += ProcessSingleRSPJ(rspj);
				REX::INFO("RSPJ / Total {} affected / With {} exception(s)", rspjCount, rspjCountExcept);
			}, stopSource.get_token(), timestamp).detach();
		}
		if (mode == LGDI) {
			lgdiCount = lgdiCountExcept = 0;
			for (auto lgdi : lgdiList) if (lgdi) lgdiCount += ProcessSingleLGDI(lgdi);
			REX::INFO("LGDI / Total {} affected / With {} exception(s)", lgdiCount, lgdiCountExcept);
		}
	}

}