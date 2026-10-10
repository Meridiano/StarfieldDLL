#pragma once

#include "hpp/utility.hpp"

namespace FCConfig {

    std::unordered_map<std::string, float, FCUtility::CIS::Hash, FCUtility::CIS::Equal> fontScaleMap{};
    std::unordered_map<std::string, bool, FCUtility::CIS::Hash, FCUtility::CIS::Equal> startupArchiveMap{};

    void ParseConfig(std::string path) {
        auto [root, data] = std::pair{ glz::generic(), std::string() };
        if (auto error = glz::read_file_json(root, path, data); error || !root.is_object()) {
            REX::INFO("Invalid JSON = {}", path);
            return;
        }
        if (static std::string fontScale{ "FontScale" }; root.contains(fontScale)) {
            if (auto& section = root.at(fontScale); section.is_object()) {
                for (auto& [key, value] : section.get_object()) if (value.is_number()) {
                    auto result = value.as<float>();
                    REX::INFO("New config option [{}] {} = {}", fontScale, FCUtility::Quoted(key), result);
                    fontScaleMap.insert_or_assign(key, result);
                }
            }
        }
        if (static std::string startupArchive{ "StartupArchive" }; root.contains(startupArchive)) {
            if (auto& section = root.at(startupArchive); section.is_object()) {
                for (auto& [key, value] : section.get_object()) if (value.is_boolean()) {
                    auto result = value.get<bool>();
                    REX::INFO("New config option [{}] {} = {}", startupArchive, FCUtility::Quoted(key), result);
                    startupArchiveMap.insert_or_assign(key, result);
                }
            }
        }
    }

    void ParseConfigList() {
        auto stringPath = "Data\\SFSE\\Plugins\\" + std::string{ SFSE::GetPluginName() };
        if (std::filesystem::path root{ stringPath }; std::filesystem::is_directory(root)) {
            for (auto& entry : std::filesystem::directory_iterator(root)) {
                if (entry.is_regular_file() && FCUtility::VerifyExtension(entry, L".json") && entry.file_size()) {
                    auto newPath = entry.path().string();
                    REX::INFO("Processing new JSON = {}", newPath);
                    ParseConfig(newPath);
                }
            }
        }
    }

}
