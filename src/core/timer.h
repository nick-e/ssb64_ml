#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <unistd.h>

namespace ssbml
{
  class timer
  {
  public:
    timer();

    void reset();
    unsigned long total_time();
    unsigned long get_delta_time();
    unsigned long get_delta_time(unsigned long min);
  private:
    std::chrono::high_resolution_clock::time_point last;
    std::chrono::high_resolution_clock::time_point start;
  };
}

#endif
