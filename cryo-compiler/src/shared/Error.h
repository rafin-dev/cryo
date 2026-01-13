#pragma once
#include <memory>
#include <vector>

namespace cryo {

    enum class ErrorSeverity {
        None = 0,
        Warning,
        Error,
        Fatal
    };

    class Error {
    public:
        virtual ~Error() = default;

        virtual void log() = 0;
        virtual ErrorSeverity get_severity() = 0;
    };

    class ErrorQueue {
    public:
        ErrorQueue() = default;
        ~ErrorQueue();

        void log() const;
        void clean();

        ErrorSeverity get_severity() const;

        template<typename ErrorType, typename... Args>
        void push_error(Args&&... args) {
            static_assert(std::is_base_of_v<Error, ErrorType>);

            m_Errors.push_back(new ErrorType(std::forward<Args>(args)...));
        }

    private:
        std::vector<Error*> m_Errors;
    };
}