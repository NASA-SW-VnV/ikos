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
 * Copyright (c) 2011-2016 United States Government as represented by the
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
#include <vector>

#include <ikos/common/types.hpp>

namespace analyzer {

// To collect statistics about the analysis
class analysis_stats {
public:
  typedef std::chrono::system_clock clock;
  typedef std::chrono::time_point< clock > time_point_t;
  typedef std::chrono::duration< float > duration_t;

public:
  class Timer {
  private:
    std::string _short_name;
    std::string _long_name;
    time_point_t _start;
    time_point_t _end;

  public:
    Timer(const std::string& short_name, const std::string& long_name)
        : _short_name(short_name), _long_name(long_name) {}

    void start() { _start = clock::now(); }

    void stop() { _end = clock::now(); }

    duration_t elapsed() const {
      return std::chrono::duration_cast< duration_t >(_end - _start);
    }

    const std::string& short_name() const { return _short_name; }

    const std::string& long_name() const { return _long_name; }
  }; // end class Timer

private:
  typedef std::vector< Timer > timers_t;

private:
  timers_t _timers;

public:
  analysis_stats() {}

  Timer& timer(const std::string& short_name) {
    for (timers_t::iterator it = _timers.begin(); it != _timers.end(); ++it) {
      if (it->short_name() == short_name) {
        return *it;
      }
    }

    throw ikos::ikos_error("timer " + short_name + " doesn't exist.");
  }

  const Timer& timer(const std::string& short_name) const {
    for (timers_t::const_iterator it = _timers.begin(); it != _timers.end();
         ++it) {
      if (it->short_name() == short_name) {
        return *it;
      }
    }

    throw ikos::ikos_error("timer " + short_name + " doesn't exist.");
  }

  Timer& start(const std::string& short_name, const std::string& long_name) {
    Timer t(short_name, long_name);
    t.start();
    _timers.push_back(t);
    return _timers.back();
  }

  Timer& stop(const std::string& short_name) {
    Timer& t = timer(short_name);
    t.stop();
    return t;
  }

  duration_t elapsed(const std::string& short_name) const {
    const Timer& t = timer(short_name);
    return t.elapsed();
  }

  void write(std::ostream& o) const {
    o << std::fixed << std::setprecision(2);
    for (timers_t::const_iterator it = _timers.begin(); it != _timers.end();
         ++it) {
      o << it->long_name() << ": " << it->elapsed().count() << std::endl;
    }
  }

  friend std::ostream& operator<<(std::ostream& o,
                                  const analysis_stats& stats) {
    stats.write(o);
    return o;
  }
}; // end class analysis_stats

} // end namespace analyzer

#endif // ANALYZER_STATS_HPP
