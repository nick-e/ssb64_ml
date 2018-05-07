#include "timer.h"

ssbml::timer::timer() :
  last(std::chrono::high_resolution_clock::now()),
  start(last)
{

}

void ssbml::timer::reset()
{
  last = std::chrono::high_resolution_clock::now();
  start = last;
}

unsigned long ssbml::timer::total_time()
{
  return (std::chrono::duration_cast<std::chrono::microseconds>(
    std::chrono::high_resolution_clock::now() - start)).count();
}

unsigned long ssbml::timer::get_delta_time()
{
  std::chrono::high_resolution_clock::time_point now
    = std::chrono::high_resolution_clock::now();
  unsigned long diff = (std::chrono::duration_cast<std::chrono::microseconds>(
    now - last)).count();
  last = now;
  return diff;
}

unsigned long ssbml::timer::get_delta_time(unsigned long min)
{
  unsigned long deltaTime = get_delta_time();
  if (deltaTime >= min)
  {
    return deltaTime;
  }
  usleep(min - deltaTime);
  return min;
}
