#pragma once

#include <chrono>

namespace gl {

    class stopwatch {
    public:

        stopwatch();
        ~stopwatch() = default;

        void reset();
        double get() const;

    private:

        std::chrono::high_resolution_clock::time_point m_start;

    };

}
