#pragma once

#include "shared/Error.h"

namespace cryo::parser {

    class ParserError : public Error {
    public:
        ParserError(std::string_view error_code, std::string error_message, std::filesystem::path file, uint32_t line_number,
            const std::string& src, uint32_t highlight_start, uint32_t highlight_size);
        ~ParserError() override = default;

        void log() override;
        ErrorSeverity get_severity() override { return m_Severity; }

    private:
        [[nodiscard]] std::pair<int, int> get_line(const std::string& file, uint32_t line_number) const;

        ErrorSeverity m_Severity;

        std::string_view m_ErrorCode;
        std::string m_Message;
        std::filesystem::path m_File;
        uint32_t m_LineNumber = 0;

        std::string m_PreviousLine;
        std::string m_Line;
        std::string m_PostLine;
        // Relative to line
        uint32_t m_HighlightStart = 0;
        uint32_t m_HighlightSize = 0;

        static std::unordered_map<std::string_view, ErrorSeverity> s_Severities;
    };

}

#define CE_INVALID_CHARACTER                      "CE1001"
#define CE_STRING_MISSING_END                     "CE1002"
#define CE_UNEXPECTED_END                         "CE1003"
#define CE_INVALID_TOKEN                          "CE1004"
#define CE_UNFINISHED_SCOPE                       "CE1005"
#define CE_EMPTY_EXPRESSION                       "CE1006"
#define CE_NO_PARENTHESIS_TO_CLOSE                "CE1007"
#define CE_UNEXPECTED_TOKEN                       "CE1008"
#define CE_DESTRUCTOR_REDEFINITION                "CE1009"
#define CE_INLINE_MEMBER_INITIALIZATION           "CE100A"