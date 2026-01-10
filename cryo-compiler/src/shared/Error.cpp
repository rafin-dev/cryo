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
        for (auto err : m_Errors) {
            delete err;
        }
        m_Errors.clear();
    }
}
