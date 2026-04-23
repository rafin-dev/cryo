#include "cryopch.h"
#include "ParserError.h"

#include <utility>

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

namespace cryo::parser {

    ParserError::ParserError(const std::string_view error_code, std::string error_message, std::filesystem::path file,
        const uint32_t line_number, const std::string &src, uint32_t highlight_start, uint32_t highlight_size)
            : m_ErrorCode(error_code), m_Message(std::move(error_message)), m_File(std::move(file)), m_LineNumber(line_number) {

        const auto ite = s_Severities.find(error_code);
        if (ite == s_Severities.end()) {
            throw std::runtime_error(std::format("Unknown Error Code {}", error_code));
        }
        m_Severity = ite->second;

        auto [line_start, line_size] = get_line(src, m_LineNumber);
        if (line_start == -1 && line_size == -1) {
            throw std::runtime_error(std::format("File {} has less than {} lines!", m_File.string(), line_number));
        }

        std::stringstream line_creator;
        line_creator << src.substr(line_start, highlight_start - line_start) <<
            RED << src.substr(highlight_start, highlight_size) << RESET <<
                src.substr(highlight_start + highlight_size, line_size - (highlight_start + highlight_size - line_start));

        m_HighlightStart = highlight_start - line_start;
        m_HighlightSize = highlight_size;

        m_Line = line_creator.str();

        if (m_LineNumber > 1) {
            auto [prev_line_start, prev_line_size] = get_line(src, m_LineNumber - 1);
            m_PreviousLine = src.substr(prev_line_start, prev_line_size);
        }

        if (auto [post_line_start, post_line_size] = get_line(src, m_LineNumber + 1); post_line_start != -1) {
            m_PostLine = src.substr(post_line_start, post_line_size);
        }
    }

    static const char* get_severity_str(const ErrorSeverity severity) {
        switch (severity) {
            case ErrorSeverity::Warning: return "[WARNING: ";
            case ErrorSeverity::Error: return "[ERROR: ";
            case ErrorSeverity::Fatal: return "[FATAL: ";
            default:
                throw std::runtime_error("Unknown Error Severity!");
        }
    }

    void ParserError::log() {
        const char* error_type = get_severity_str(m_Severity);
        std::cout << RED << error_type << m_ErrorCode << "]" RESET " at file: " << m_File << std::endl;

        if (!m_PreviousLine.empty()) {
            std::cout << m_LineNumber - 1 << " | " << m_PreviousLine << std::endl;
        }

        const std::string msg = std::format("{} | ", m_LineNumber);
        std::cout << msg << m_Line << std::endl;

        for (uint32_t i = 0; i < m_HighlightStart + msg.size(); i++) {
            std::cout << " ";
        }
        std::cout << YELLOW;
        for (int i = 0; i < m_HighlightSize; i++) {
            std::cout << "^";
        }
        std::cout << RESET << std::endl;

        if (!m_PostLine.empty()) {
            std::cout << m_LineNumber + 1 << " | " << m_PostLine << std::endl;
        }

        std::cout << "-- Message: " << m_Message << " --" << std::endl << std::endl;
    }

    std::pair<int, int> ParserError::get_line(const std::string &file, uint32_t line_number) const {
        int line_start = 0;
        int line_size = 0;
        bool found_line = false;
        for (int i = 0, line = 1; i < file.size(); i++) {
            if (file[i] == '\n') {
                line++;
                if (line == line_number) {
                    line_start = i + 1;
                }
            }

            if (line == line_number) {
                found_line = true;
                line_size++;
            } else if (line == line_number + 1) {
                break;
            }
        }
        line_size--;

        return found_line ? std::pair(line_start, line_size) : std::pair(-1, -1);
    }

    std::unordered_map<std::string_view, ErrorSeverity> ParserError::s_Severities = {
        { CE_INVALID_CHARACTER,                      ErrorSeverity::Error },
        { CE_STRING_MISSING_END,                     ErrorSeverity::Error },
        { CE_UNEXPECTED_END,                         ErrorSeverity::Error },
        { CE_INVALID_TOKEN,                          ErrorSeverity::Error },
        { CE_UNFINISHED_SCOPE,                       ErrorSeverity::Error },
        { CE_EMPTY_EXPRESSION,                       ErrorSeverity::Error },
        { CE_NO_PARENTHESIS_TO_CLOSE,                ErrorSeverity::Error },
        { CE_UNEXPECTED_TOKEN,                       ErrorSeverity::Error },
        { CE_DESTRUCTOR_REDEFINITION,                ErrorSeverity::Error },
        { CE_INLINE_MEMBER_INITIALIZATION,           ErrorSeverity::Error },
    };
}
