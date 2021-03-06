//
// Created by XingfengYang on 2020/1/2.
//
#include "../../include/timer/TimeKeeper.h"

namespace Timer {
    TimeKeeper::~TimeKeeper() noexcept = default;

    TimeKeeper::TimeKeeper(Timer::TimeKeeper &&) noexcept = default;

    TimeKeeper &TimeKeeper::operator=(TimeKeeper &&) noexcept = default;

    TimeKeeper::TimeKeeper() : impl(std::make_shared<TimeKeeperImpl>()) {
    }

    double TimeKeeper::GetCurrentTime() {
        return impl->GetCurrentTime();
    }
}
