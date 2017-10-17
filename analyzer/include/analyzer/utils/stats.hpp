/*******************************************************************************
 *
 * Collect and print statistics from analyses.
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2017 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Disclaimers:
 *
 * No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
 * ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS,
 * ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE
 * ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO
 * THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
 * ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
 * RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
 * RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
 * DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE,
 * IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."
 *
 * Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST
 * THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL
 * AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS
 * IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH
 * USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM,
 * RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD
 * HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS,
 * AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.
 * RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE,
 * UNILATERAL TERMINATION OF THIS AGREEMENT.
 *
 ******************************************************************************/

#ifndef ANALYZER_STATS_HPP
#define ANALYZER_STATS_HPP

#include <chrono>
#include <iomanip>
#include <string>
#include <unordered_map>

#include <ikos/common/types.hpp>

#include <analyzer/analysis/common.hpp>

namespace analyzer {

// To collect statistics about the analysis
class analysis_stats {
public:
  typedef std::chrono::steady_clock clock;
  typedef std::chrono::time_point< clock > time_point_t;
  typedef std::chrono::duration< double > duration_t;

public:
  class Timer {
  private:
    std::string _name;
    time_point_t _start;
    time_point_t _end;

  public:
    Timer(const std::string& name) : _name(name) {}

    void start() { _start = clock::now(); }

    void stop() { _end = clock::now(); }

    duration_t elapsed() const {
      return std::chrono::duration_cast< duration_t >(_end - _start);
    }

    const std::string& name() const { return _name; }

  }; // end class Timer

private:
  typedef std::unordered_map< std::string, Timer > timers_t;

private:
  timers_t _timers;

public:
  analysis_stats() {}

  Timer& timer(const std::string& name) {
    auto it = _timers.find(name);

    if (it != _timers.end()) {
      return it->second;
    } else {
      throw analyzer_error("timer " + name + " does not exist");
    }
  }

  const Timer& timer(const std::string& name) const {
    auto it = _timers.find(name);

    if (it != _timers.end()) {
      return it->second;
    } else {
      throw analyzer_error("timer " + name + " does not exist");
    }
  }

  Timer& start(const std::string& name) {
    Timer t(name);
    t.start();
    auto res = _timers.insert(timers_t::value_type(name, t));
    return res.first->second;
  }

  Timer& stop(const std::string& name) {
    Timer& t = timer(name);
    t.stop();
    return t;
  }

  duration_t elapsed(const std::string& name) const {
    const Timer& t = timer(name);
    return t.elapsed();
  }

}; // end class analysis_stats

} // end namespace analyzer

#endif // ANALYZER_STATS_HPP
