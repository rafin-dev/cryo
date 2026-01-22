#include "cryopch.h"
#include "Error.h"

namespace cryo {

    ErrorQueue::~ErrorQueue() {
        clean();
    }

    void ErrorQueue::log() const {
        for (const auto& error : m_Errors) {
            error->log();
        }
    }

    void ErrorQueue::clean() {
        for (const auto err : m_Errors) {
            delete err;
        }
        m_Errors.clear();
    }

    ErrorSeverity ErrorQueue::get_severity() const {
        auto sv = ErrorSeverity::None;
        for (auto& error : m_Errors) {
            if (error->get_severity() > sv) {
                sv = error->get_severity();
            }
        }
        return sv;
    }

    void ErrorQueue::push_ErrorQueue(ErrorQueue &queue) {
        m_Errors.append_range(queue.m_Errors);
        queue.m_Errors.clear();
    }
}
