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

#ifndef TRAACT_PERFORMANCEMONITOR_H
#define TRAACT_PERFORMANCEMONITOR_H

#include <string>
#include <iostream>
#include <vector>
#include <traact/datatypes.h>

namespace traact::util {

class PerformanceMonitor {
 public:

  class BlockTimer {
   public:

    BlockTimer(PerformanceMonitor &m_monitor,
               size_t index,
               const char *sBlockName,
               const char *sCodeFile,
               unsigned nCodeLine)
        : m_index(index), m_monitor(m_monitor), m_startTime(now()) {

      if (!m_monitor.initialized(m_index))
        m_monitor.initializeTimeBlock(m_index, sBlockName, sCodeFile, nCodeLine);
    }

    ~BlockTimer() {
      m_monitor.addMeasurement(m_index, now() - m_startTime);
    }

    BlockTimer(const BlockTimer &) = delete;

    BlockTimer &operator=(BlockTimer &) = delete;

   protected:
    size_t m_index;
    PerformanceMonitor &m_monitor;
    TimestampType m_startTime;
  };

  explicit PerformanceMonitor(std::string name);

  ~PerformanceMonitor();

  void addMeasurement(size_t index, TimeDurationType ticks);

  const std::string &getName() const;

  bool initialized(size_t index) const;

  void initializeTimeBlock(size_t index, const char *sBlockName, const char *sCodeFile, unsigned nCodeLine);

  std::string toString() const;

  TimeDurationType getTotalTime() const;

  size_t getTimeBlockCount() const;

  TimeDurationType getTotalTimeForBlock(size_t index) const;

  TimeDurationType getAverageTimeForBlock(size_t index) const;

  void reset();

 protected:

  struct TimeBlock {
    TimeBlock() : initialized(
        false), codeLine(0), blockName(""), codeFile(""), index(0), runs(0), deltaTime(0) {};
    TimeBlock(size_t index, unsigned int codeLine, const char *name, std::string codeFile) : initialized(
        true),
                                                                                             codeLine(codeLine),
                                                                                             blockName(name),
                                                                                             codeFile(std::move(codeFile)),
                                                                                             index(index),
                                                                                             runs(0),
                                                                                             deltaTime(0) {}

    bool initialized;
    unsigned codeLine;
    std::string codeFile;
    std::string blockName;

    size_t index;

    size_t runs;
    TimeDurationType deltaTime;
  };

  std::vector<TimeBlock> m_blockTimer;

  const std::string m_name;

  TimestampType m_startTime;
};

#define MEASURE_TIME(monitor, index, name) traact::util::PerformanceMonitor::BlockTimer _timeVar_##timer( monitor, index, name, __FILE__, __LINE__ );

}

#endif //TRAACT_PERFORMANCEMONITOR_H
