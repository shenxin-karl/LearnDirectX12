#pragma once
#include <memory>
#include <source_location>
#include <string>
#include <Imgui/imgui.h>
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
}


class LogSystemEditor : public core::LogSystem {
public:
    LogSystemEditor(const std::string &fileName);
    ~LogSystemEditor();
    void drawLogWindow();
    void clear();
    void setShowWindow(bool bShow);
    void updateCurrentTime();
    void output(const std::source_location &location, Log::LogLevel level, const std::string &output);
private:
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
        template<size_t N>
        constexpr FormatWithLocation(const char (&fmt)[N], const std::source_location &sl = std::source_location::current()) : _fmt(fmt), _location(sl) {}
        constexpr FormatWithLocation(std::string_view fmt, const std::source_location &sl = std::source_location::current()) : _fmt(fmt), _location(sl) {}
    public:
	    std::string_view     _fmt{};
        std::source_location _location{};
    };

    template<typename ...Args>
    void info(FormatWithLocation fmt, Args&&...args) {
        std::string output = std::vformat(fmt._fmt, std::make_format_args(args...));
        static_cast<LogSystemEditor *>(core::LogSystem::instance())->output(fmt._location, Info, output);
    }

    template<typename ...Args>
    void debug(FormatWithLocation fmt, Args&&...args) {
        std::string output = std::vformat(fmt._fmt, std::make_format_args(args...));
        static_cast<LogSystemEditor *>(core::LogSystem::instance())->output(fmt._location, Debug, output);
    }

    template<typename ...Args>
    void warning(FormatWithLocation fmt, Args&&...args) {
        std::string output = std::vformat(fmt._fmt, std::make_format_args(args...));
        static_cast<LogSystemEditor *>(core::LogSystem::instance())->output(fmt._location, Warning, output);
    }

    template<typename ...Args>
    void error(FormatWithLocation fmt, Args&&...args) {
        std::string output = std::vformat(fmt._fmt, std::make_format_args(args...));
        static_cast<LogSystemEditor *>(core::LogSystem::instance())->output(fmt._location, Error, output);
    }

}

}
