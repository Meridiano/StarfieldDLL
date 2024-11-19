#pragma once

#include "hpp/utility.hpp"

static std::vector<std::string>                                        dataLoadedMap;
static std::map<int, std::vector<std::string>>                         gameLoadedMap;
static std::map<std::pair<std::string, int>, std::vector<std::string>> menuActionMap;

namespace CCRFunctions {

    void StoreCommands() {
        std::string path = "Data/SFSE/Plugins/ConsoleCommandRunner";
        if (fs::exists(path)) {
            std::string type = ".toml";
            for (fs::directory_entry fileEntry : fs::directory_iterator(path)) {
                fs::path filePath = fileEntry.path();
                if (fs::is_regular_file(filePath) && filePath.extension() == type) {
                    auto fileName = filePath.filename().string();
                    logs::info("Reading config file >> {}", fileName);
                    toml::parse_result data;
                    try {
                        data = toml::parse_file(filePath.string());
                    } catch (...) {
                        logs::info("Parsing error, file name >> {}", fileName);
                        continue;
                    }
                    toml::array* events = data.get_as<toml::array>("Event");
                    if (events) for (toml::node& event : *events) {
                        toml::table eventTable = *event.as_table();
                        std::string eventType = eventTable["EventType"].value_or<std::string>("");
                        if (eventType == "DataLoaded") {
                            toml::array* comArray = eventTable["Commands"].as_array();
                            if (comArray) for (toml::node& com : *comArray) {
                                std::string command = com.as_string()->get();
                                logs::info("{} store >> {}", eventType, command);
                                dataLoadedMap.push_back(command);
                            }
                        } else if (eventType == "GameLoaded") {
                            auto loadTypeData = eventTable["aiLoadType"].as_integer();
                            int loadType = loadTypeData ? loadTypeData->get() : 0;
                            toml::array* comArray = eventTable["Commands"].as_array();
                            if (comArray) for (toml::node& com : *comArray) {
                                std::string command = com.as_string()->get();
                                logs::info("{} with options [ {} ] store >> {}", eventType, loadType, command);
                                if (gameLoadedMap.contains(loadType)) gameLoadedMap.at(loadType).push_back(command);
                                else gameLoadedMap.insert_or_assign(loadType, std::vector(1, command));
                            }
                        } else if (eventType == "OnMenuOpenCloseEvent") {
                            auto menuNameData = eventTable["asMenuName"].as_string();
                            auto isOpeningData = eventTable["abOpening"].as_boolean();
                            std::string menuName = menuNameData ? menuNameData->get() : "";
                            int isOpening =  isOpeningData ? isOpeningData->get() : -1;
                            std::pair menuAction(menuName, isOpening);
                            toml::array* comArray = eventTable["Commands"].as_array();
                            if (comArray) for (toml::node& com : *comArray) {
                                std::string command = com.as_string()->get();
                                logs::info("{} with options [ {} {} ] store >> {}", eventType, menuName, isOpening, command);
                                if (menuActionMap.contains(menuAction)) menuActionMap.at(menuAction).push_back(command);
                                else menuActionMap.insert_or_assign(menuAction, std::vector(1, command));
                            }
                        }
                    }
                }
            }
        }
    }

    void RunDataCommands() {
        for (std::string command : dataLoadedMap) {
            if (command.empty()) continue;
            logs::info("{} execute >> {}", "DataLoaded", command);
            CCRUtility::ConsoleExecute(command);
        }
    }

    void RunGameCommands(int loadType) {
        if (gameLoadedMap.contains(loadType)) {
            auto commandArray = gameLoadedMap.at(loadType);
            for (std::string command : commandArray) {
                if (command.empty()) continue;
                logs::info("{} with options [ {} ] execute >> {}", "GameLoaded", loadType, command);
                CCRUtility::ConsoleExecute(command);
            }
        }
    }

    void RunMenuCommands(std::string sName, int iOpen) {
        for (auto mapEntry : menuActionMap) {
            auto menuAction = mapEntry.first;
            std::string menuName = menuAction.first;
            int isOpening = menuAction.second;
            bool matchName = (menuName == sName || menuName == "");
            bool matchOpening = (isOpening == iOpen || isOpening == -1);
            if (matchName && matchOpening) {
                auto commandArray = mapEntry.second;
                for (std::string command : commandArray) {
                    if (command.empty()) continue;
                    logs::info("{} with options [ {} {} ] execute >> {}", "OnMenuOpenCloseEvent", menuName, isOpening, command);
                    CCRUtility::ConsoleExecute(command);
                }
            }
        }
    }

}