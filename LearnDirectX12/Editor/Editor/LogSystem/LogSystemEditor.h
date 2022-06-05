#pragma once
#include <memory>
#include <source_location>
#include <string>
#include <Imgui/imgui.h>
#include "Editor/IEditorItem.h"
#include <format>
#include <mutex>
#include "Core/LogSystem/LogSystem.h"

namespace ED {

namespace Log {
    enum LogLevel : size_t {
        Info = 0,
        Debug = 1,
        Warning = 2,
        Error = 3,
    };
    template<LogLevel Level, typename ...Args>
    struct LogOutputHelper;
}


class LogSystemEditor : public core::LogSystem, public IEditorWindow {
public:
    LogSystemEditor(const std::string &fileName);
    void showWindow() override;
    void clear();
    void setShowWindow(bool bShow);
    void updateCurrentTime();
private:
    template<Log::LogLevel, typename...Args>
    friend struct Log::LogOutputHelper;

    ImGuiTextBuffer     _buf;
    ImGuiTextFilter     _filter;
    ImVector<int>       _lineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
    std::locale         _localeFormat;
    std::string         _currentTime;
    bool                _autoScroll    = true;  // Keep scrolling if already at the bottom.
    bool                _showLogWindow = true;
};

namespace Log {

    struct FormatWithLocation {
        FormatWithLocation(const char *fmt, const std::source_location &sl = std::source_location::current()) : _fmt(fmt), _location(sl) {}
        FormatWithLocation(std::string_view fmt, const std::source_location &sl = std::source_location::current()) : _fmt(fmt), _location(sl) {}
    public:
	    std::string_view     _fmt;
        std::source_location _location;
    };

    template<LogLevel Level, typename ...Args>
    struct LogOutputHelper {
        LogOutputHelper(FormatWithLocation fmt, Args&&...args) {
	        constexpr std::string_view sPrefixMessage[] = {
                "[-Info--]",
                "[-Debug-]",
                "[Warning]",
                "[-Error-]",
	        };

            thread_local static std::string msg;
            msg.clear();

            std::string shortFileName = fmt._location.file_name();
            auto pos = shortFileName.find_last_of("\\\\");
            if (pos != std::string::npos)
                shortFileName = shortFileName.substr(pos+1);

            auto pEditorLog = static_cast<LogSystemEditor *>(LogSystemEditor::instance());
            msg += std::format(" {}:{} ", shortFileName, fmt._location.line());
            msg += std::format(fmt._fmt, std::forward<Args>(args)...);
            msg += '\n';
            LogSystemEditor::instance()->append(msg);

	        {
				static std::mutex mutex;
                std::unique_lock lock(mutex);
                pEditorLog->_buf.append(pEditorLog->_currentTime.c_str());
                pEditorLog->_buf.append(sPrefixMessage[Level].data());
                int oldSize = pEditorLog->_buf.size();
                pEditorLog->_buf.append(msg.c_str());
                for (size_t i = 0; i < msg.length(); ++i) {
                    if (msg[i] == '\n') {
	                    int offset = oldSize + static_cast<int>(i) + 1;
						pEditorLog->_lineOffsets.push_back(offset);
                    }
                }
	        }
        }
    };

    template<typename ...Args>
    auto info(FormatWithLocation fmt, Args&&...args) -> LogOutputHelper<Log::Info, Args...> {
        return { fmt, std::forward<Args>(args)... };
    }

    template<typename ...Args>
    auto debug(FormatWithLocation fmt, Args&&...args) -> LogOutputHelper<Log::Debug, Args...> {
        return { fmt, std::forward<Args>(args)... };
    }

    template<typename ...Args>
    auto warning(FormatWithLocation fmt, Args&&...args) -> LogOutputHelper<Log::Warning, Args...> {
        return { fmt, std::forward<Args>(args)... };
    }

    template<typename ...Args>
    auto error(FormatWithLocation fmt, Args&&...args) -> LogOutputHelper<Log::Error, Args...> {
        return { fmt, std::forward<Args>(args)... };
    }

}

}
