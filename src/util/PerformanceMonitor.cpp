/*  BSD 3-Clause License
 *
 *  Copyright (c) 2020, FriederPankratz <frieder.pankratz@gmail.com>
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cassert>
#include "traact/util/PerformanceMonitor.h"

traact::util::PerformanceMonitor::PerformanceMonitor(std::string name, int block_timer_count) : m_name(std::move(name)), m_startTime(now()) {
  m_blockTimer.resize(block_timer_count);
}

traact::util::PerformanceMonitor::~PerformanceMonitor() = default;

void traact::util::PerformanceMonitor::addMeasurement(size_t index, traact::TimeDurationType ticks) {
  m_blockTimer[index].deltaTime += ticks;
  m_blockTimer[index].runs++;
}

const std::string &traact::util::PerformanceMonitor::getName() const {
  return m_name;
}

bool traact::util::PerformanceMonitor::initialized(size_t index) const {
	assert(index <= m_blockTimer.size());
  return m_blockTimer[index].initialized;
}

void traact::util::PerformanceMonitor::initializeTimeBlock(size_t index,
                                                           const char *sBlockName,
                                                           const char *sCodeFile,
                                                           unsigned nCodeLine) {
  if (m_blockTimer.size() <= index) {
    m_blockTimer.resize(index + 1);
  }

  m_blockTimer[index] = TimeBlock(index, nCodeLine, sBlockName, sCodeFile);
}

std::string traact::util::PerformanceMonitor::toString() const {

  using nanoMilliseconds = std::chrono::duration<float, std::milli>;
  using nanoSeconds = std::chrono::duration<float>;

  TimeDurationType totalRunTime = getTotalTime();

  std::stringstream ss;

  ss << std::endl << "Result for " << getName() << std::endl;
  ss << "totalRuntime: " << std::setw(7) << nanoMilliseconds(totalRunTime).count() << "ms" << std::endl;

  TimeDurationType totalBlockTime;
  for (const auto &timer : m_blockTimer) {
    totalBlockTime += timer.deltaTime;
  }

  for (size_t index = 0; index < m_blockTimer.size(); ++index) {

    const TimeBlock &tmp = m_blockTimer[index];

      if (tmp.deltaTime == TimeDurationType(0) || tmp.runs == 0) {
          //ss << " no data, runs " << tmp.runs << ", nanoseconds " << tmp.deltaTime.count() << std::endl;
          continue;
      }

    ss << std::setw(40) << tmp.blockName << " : ";



    ss << " total time: " << std::setw(10) << nanoMilliseconds(getTotalTimeForBlock(index)).count() << "ms";
    ss << ", runs: " << std::setw(6) << tmp.runs;

    ss << ", avg time: " << std::setw(10) << nanoMilliseconds(getAverageTimeForBlock(index)).count() << "ms";

    double d = tmp.deltaTime.count();
    double t = totalRunTime.count();
    double percentage = d / t;

    ss << ", of total time: " << std::setw(10) << percentage * 100 << "%";

    float seconds = nanoSeconds(totalRunTime).count();
    if (seconds > 0) {
      ss << ", call per second: " << std::setw(7) << tmp.runs / seconds << std::endl;
    } else {
      ss << std::endl;
    }
  }

  return ss.str();
}

void traact::util::PerformanceMonitor::reset() {

  m_startTime = now();
  for (auto &item : m_blockTimer) {
    item.runs = 0;
    item.deltaTime = TimeDurationType(0);
  }

}

traact::TimeDurationType traact::util::PerformanceMonitor::getTotalTime() const {
  return now() - m_startTime;
}

size_t traact::util::PerformanceMonitor::getTimeBlockCount() const {
  return m_blockTimer.size();
}

traact::TimeDurationType traact::util::PerformanceMonitor::getTotalTimeForBlock(size_t index) const {
  return m_blockTimer[index].deltaTime;

}

traact::TimeDurationType traact::util::PerformanceMonitor::getAverageTimeForBlock(size_t index) const {
  return m_blockTimer[index].deltaTime / m_blockTimer[index].runs;
}
