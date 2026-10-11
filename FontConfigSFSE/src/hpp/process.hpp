#pragma once

#include "hpp/data.hpp"

namespace FCProcess {

	void AdjustChars(FCData::BSScaleformManager* scaleform) {
		auto oldChars = FCUtility::Dequoted(FCData::validNameChars);
		auto newChars = FCUtility::Dequoted(scaleform->validNameChars.data());
		if (oldChars != newChars) {
			newChars = FCUtility::Quoted(oldChars + newChars);
			FCData::validNameChars = scaleform->validNameChars = newChars.data();
		}
	}

	std::pair<char*, std::size_t> AdjustArchives(char* old) {
		std::string result{ old };
		std::string addition{ "" };
		static std::string delimiter{ ", " };
		for (auto& [archive, enabled] : FCConfig::startupArchiveMap)
			if (enabled)
				addition += (delimiter + archive);
		result += addition;
		return { FCUtility::MakeCopy(&result)->data(), result.size() };
	}

	void ApplyFontScale(FCData::FontMapData** mapHolder) {
		if (mapHolder) if (auto map = *mapHolder; map) {
			REX::INFO("Map address = {:X}", (std::uint64_t)map);
			std::uint64_t index = 0;
			std::span<FCData::FontMapEntry> span{ &map->firstEntry, map->maxIndex + 1 };
			for (auto& entry : span) if (entry.flags != -2) {
				auto keyData = entry.key.GetData();
				auto tagData = entry.tag.GetData();
				if (keyData && tagData) {
					bool scaleChange = false;
					float oldScale = entry.scale;
					static auto mapEnd = FCConfig::fontScaleMap.end();
					if (auto mapEntry = FCConfig::fontScaleMap.find(keyData); mapEntry != mapEnd) {
						entry.scale = mapEntry->second;
						scaleChange = true;
					}
					REX::INFO(
						"Entry #{} / {} = {} / Scale {} {}",
						index++, FCUtility::Quoted(keyData), FCUtility::Quoted(tagData),
						scaleChange ? std::format("changed from {} to", oldScale) : "is", entry.scale
					);
				}
			}
		}
	}

	void ExtendedProcess(FCData::BSScaleformManager* scaleform, REL::ID funcID) {
		// save valid chars
		FCData::validNameChars = scaleform->validNameChars;
		REX::INFO("Original valid chars size = {}", FCData::validNameChars.size());
		// relocate
		using type = void(*)(decltype(scaleform));
		static REL::Relocation<type> function{ funcID };
		// iterate configs
		auto subPath = "Data\\SFSE\\Plugins\\" + std::string{ SFSE::GetPluginName() };
		if (std::filesystem::path root{ subPath }; std::filesystem::is_directory(root)) {
			for (auto& entry : std::filesystem::directory_iterator(root)) {
				if (entry.is_regular_file() && FCUtility::VerifyExtension(entry, L".txt") && entry.file_size()) {
					auto newName = entry.path().filename().string();
					FCData::configs.push_back(newName);
					FCData::functionPass += 1;
					function(scaleform);
					AdjustChars(scaleform);
				}
			}
		}
		// final tweaks
		REX::INFO("New valid chars size = {}", scaleform->validNameChars.size());
		if (FCData::fontMap) ApplyFontScale(FCData::fontMap->mapHolder);
	}

}
