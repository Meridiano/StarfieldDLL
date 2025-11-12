using procType = bool(*)(const char*, char**);
procType procFunc = nullptr;
bool ready = false;

template<typename Func>
auto WriteFunctionHook(std::uint64_t id, std::size_t copyCount, Func destination) {
    const auto target = REL::ID(id).address();
    if (REL::Pattern<"E8">().match(target)) return TRAMPOLINE.write_call<5>(target, destination);
    if (REL::Pattern<"E9">().match(target)) return TRAMPOLINE.write_jmp<5>(target, destination);
    struct XPatch: Xbyak::CodeGenerator {
        using ull = unsigned long long;
        using uch = unsigned char;
        uch workspace[64];
        XPatch(std::uintptr_t baseAddress, ull bytesCount): Xbyak::CodeGenerator(bytesCount + 14, workspace) {
            auto bytePtr = reinterpret_cast<uch*>(baseAddress);
            for (ull i = 0; i < bytesCount; i++) db(*bytePtr++);
            jmp(qword[rip]);
            dq(ull(bytePtr));
        }
    };
    XPatch patch(target, copyCount);
    patch.ready();
    auto patchSize = patch.getSize();
    TRAMPOLINE.write_jmp<5>(target, destination);
    auto alloc = TRAMPOLINE.allocate(patchSize);
    std::memcpy(alloc, patch.getCode(), patchSize);
    return reinterpret_cast<std::uintptr_t>(alloc);
}

std::string TryTextReplacement(std::string input) {
    if (ready) {
        char* resultData = nullptr;
        auto success = procFunc(input.data(), &resultData);
        if (success && resultData) {
            std::string result{ resultData };
            std::free(resultData);
            return result;
        }
    }
    return input;
}

void LogCommandData(const char* oldData, const char* newData) {
    REX::INFO("--- Console command block ---");
    REX::INFO("Old command = {}", oldData);
    REX::INFO("New command = {}", newData);
}

class BatchExecutionHook {
private:
    struct Call {
        static void NEW(RE::BSStringPool::Entry** entryHandler, const char* command, bool caseSensitive) {
            if (command) {
                auto newCommand = TryTextReplacement(command);
                auto newCommandData = newCommand.data();
                LogCommandData(command, newCommandData);
                return OLD(entryHandler, newCommandData, caseSensitive);
            }
            OLD(entryHandler, command, caseSensitive);
        }
        static inline REL::Relocation<decltype(NEW)> OLD;
    };
public:
    static void Install() {
        REL::Relocation target{ REL::ID(113575), 0x126 };
        Call::OLD = target.write_call<5>(Call::NEW);
        REX::INFO("Batch execution hook installed");
    }
};

class ConsoleExecutionHook {
private:
    struct Prologue {
        static void NEW(void* manager, const char* command) {
            if (command) {
                auto newCommand = TryTextReplacement(command);
                auto newCommandData = newCommand.data();
                LogCommandData(command, newCommandData);
                return OLD(manager, newCommandData);
            }
            OLD(manager, command);
        }
        static inline REL::Relocation<decltype(NEW)> OLD;
    };
public:
    static void Install() {
        // 48 89 54 24 10
        Prologue::OLD = WriteFunctionHook(113576, 5, Prologue::NEW);
        REX::INFO("Console execution hook installed");
    }
};

void MessageCallback(SFSE::MessagingInterface::Message* a_msg) {
    if (a_msg->type == SFSE::MessagingInterface::kPostPostLoad) {
        auto ccr = REX::W32::GetModuleHandleW(L"ConsoleCommandRunner");
        if (ccr) {
            auto api = REX::W32::GetProcAddress(ccr, "TextReplacementAPI");
            if (api) {
                procFunc = reinterpret_cast<procType>(api);
                REX::INFO("Text replacement address = {:X}", (std::uint64_t)procFunc);
            } else REX::FAIL("TextReplacementAPI export not found");
        } else REX::FAIL("ConsoleCommandRunner module not found");
        // install hooks
        BatchExecutionHook::Install();
        ConsoleExecutionHook::Install();
    } else if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
        if (procFunc) ready = true;
    }
}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
    SFSE::InitInfo info{
        .logPattern = "%d.%m.%Y %H:%M:%S [%s:%#] %v",
        .trampoline = true,
        .trampolineSize = 80
    };
    SFSE::Init(a_sfse, info);

    const auto gameInfo = a_sfse->RuntimeVersion().string(".");
    REX::INFO("Starfield v{}", gameInfo);

    const auto messagingInterface = SFSE::GetMessagingInterface();
    if (messagingInterface && messagingInterface->RegisterListener(MessageCallback)) {
        REX::INFO("Message listener registered");
    } else {
        REX::FAIL("Message listener not registered");
    }
    return true;
}
