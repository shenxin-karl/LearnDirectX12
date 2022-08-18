#include "Editor/EditorStd.h"
#include "LogSystemEditor.h"
#include "Editor/Editor.h"
#include "Editor/MenuBar/EditorMenuBar.h"
#include "Imgui/imgui.h"

namespace ED {

LogSystemEditor::LogSystemEditor(const std::string &fileName) : LogSystem(fileName), _localeFormat("ja_JP.utf8") {
	clear();

    auto &pMainMenuBar = Editor::instance()->pMainMenuBar;
    Menu *pWindowMenu = pMainMenuBar->registerBarItem(MainMenuBar::Windows);
    pWindowMenu->addSubItemGroup("Log")->menuItems = [&]() {
        ImGui::MenuItem("Log", nullptr, &_showLogWindow);
    };
}

LogSystemEditor::~LogSystemEditor() {
    auto &pMainMenuBar = Editor::instance()->pMainMenuBar;
    Menu *pWindowMenu = pMainMenuBar->getBarItem(MainMenuBar::Windows);
    pWindowMenu->removeSubItemGroupByName("Log");
}

void LogSystemEditor::drawLogWindow() {
    if (!_showLogWindow)
        return;

    if (!ImGui::Begin("Log Window", &_showLogWindow)) {
        ImGui::End();
        return;
    }

    // Options menu
    if (ImGui::BeginPopup("Options")) {
        ImGui::Checkbox("Auto-scroll", &_autoScroll);
        ImGui::EndPopup();
    }

    // Main window
    if (ImGui::Button("Options"))
        ImGui::OpenPopup("Options");
    ImGui::SameLine();
    bool bClear = ImGui::Button("Clear");
    ImGui::SameLine();
    bool bCopy = ImGui::Button("Copy");
    ImGui::SameLine();
    //bool bSave = ImGui::Button("Save");


    _filter.Draw("Filter", -100.0f);

    ImGui::Separator();
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    if (bClear)
        clear();
    if (bCopy)
        ImGui::LogToClipboard();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    const char *buf = _buf.begin();
    const char *buf_end = _buf.end();

    static ImVec4 sLogColor[4] = {
        ImVec4(1.f, 1.f, 1.f, 1.f),
        ImVec4(0.5f, 1.f, 0.f, 1.f),
        ImVec4(0.f, 1.f, 1.f, 1.f),
        ImVec4(1.f, 0.f, 0.f, 1.f),
    };

    static auto textUnformatted = [&](const char *line_start, const char *line_end) {
        size_t offset = _currentTime.length() + 2;
        size_t length = line_end - line_start;
        if (length < offset) {
            ImGui::TextUnformatted(line_start, line_end);
            return;
        }

        switch (line_start[offset]) {
        case 'D':
            ImGui::PushStyleColor(ImGuiCol_Text, sLogColor[Log::LogLevel::Debug]);
            break;
        case 'a':
            ImGui::PushStyleColor(ImGuiCol_Text, sLogColor[Log::LogLevel::Warning]);
            break;
        case 'E':
            ImGui::PushStyleColor(ImGuiCol_Text, sLogColor[Log::LogLevel::Error]);
            break;
        case 'I':
        default:
            ImGui::PushStyleColor(ImGuiCol_Text, sLogColor[Log::LogLevel::Info]);
            break;
        }
        ImGui::TextUnformatted(line_start, line_end);
        ImGui::PopStyleColor();
    };

    if (_filter.IsActive())
    {
        // In this example we don't use the clipper when Filter is enabled.
        // This is because we don't have a random access on the result on our filter.
        // A real application processing logs with ten of thousands of entries may want to store the result of
        // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
        for (int line_no = 0; line_no < _lineOffsets.Size; line_no++)
        {
            const char *line_start = buf + _lineOffsets[line_no];
            const char *line_end = (line_no + 1 < _lineOffsets.Size) ? (buf + _lineOffsets[line_no + 1] - 1) : buf_end;
            if (_filter.PassFilter(line_start, line_end))
                textUnformatted(line_start, line_end);
        }
    } else {
        // The simplest and easy way to display the entire buffer:
        //   ImGui::TextUnformatted(buf_begin, buf_end);
        // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
        // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
        // within the visible area.
        // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
        // on your side is recommended. Using ImGuiListClipper requires
        // - A) random access into your data
        // - B) items all being the  same height,
        // both of which we can handle since we an array pointing to the beginning of each line of text.
        // When using the filter (in the block of code above) we don't have random access into the data to display
        // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
        // it possible (and would be recommended if you want to search through tens of thousands of entries).
        ImGuiListClipper clipper;
        clipper.Begin(_lineOffsets.Size);
        while (clipper.Step())
        {
            for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
            {
                const char *line_start = buf + _lineOffsets[line_no];
                const char *line_end = (line_no + 1 < _lineOffsets.Size) ? (buf + _lineOffsets[line_no + 1] - 1) : buf_end;
                textUnformatted(line_start, line_end);
            }
        }
        clipper.End();
    }
    ImGui::PopStyleVar();

    if (_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}


void LogSystemEditor::clear() {
    _buf.clear();
    _lineOffsets.clear();
    _lineOffsets.push_back(0);
}

void LogSystemEditor::setShowWindow(bool bShow) {
    _showLogWindow = bShow;
}

void LogSystemEditor::updateCurrentTime() {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

	std::stringstream sbuf;
    sbuf.imbue(_localeFormat);
    sbuf << std::put_time(&tm, "%c");
    _currentTime = std::format("[{}]", sbuf.str());
}

void LogSystemEditor::output(const std::source_location &location, Log::LogLevel level, const std::string &output) {
    constexpr std::string_view sPrefixMessage[] = {
	    "[-Info--]",
	    "[-Debug-]",
	    "[Warning]",
	    "[-Error-]",
    };

    thread_local static std::string msg;
    msg.clear();

    std::string shortFileName = location.file_name();
    auto pos = shortFileName.find_last_of("\\\\");
    if (pos != std::string::npos)
        shortFileName = shortFileName.substr(pos + 1);

    msg += std::format(" {}:{} ", shortFileName, location.line());
    msg += output;
    msg += '\n';
    LogSystemEditor::instance()->append(msg);

    {
        static std::mutex mutex;
        std::unique_lock lock(mutex);
        _buf.append(_currentTime.c_str());
        _buf.append(sPrefixMessage[level].data());
        int oldSize = _buf.size();
        _buf.append(msg.c_str());
        for (size_t i = 0; i < msg.length(); ++i) {
            if (msg[i] == '\n') {
                int offset = oldSize + static_cast<int>(i) + 1;
                _lineOffsets.push_back(offset);
            }
        }
    }
}

}
