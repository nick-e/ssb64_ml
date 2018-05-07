#ifndef TRAIN_SESSION_H
#define TRAIN_SESSION_H

#include <string>
#include <thread>
#include <mutex>
#include <iostream>
#include <atomic>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <iomanip>
#include <errno.h>
#include <cstdlib>
#include <stdio.h>

#include <gtkmm-3.0/gtkmm.h>

#include "video_file.h"
#include "timer.h"
#include "util.h"
#include "gamepad_file.h"
#include "child_program.h"

namespace ssbml
{
  class train_session
  {
  public:
    struct info
    {
      bool modelLoaded;
      bool trainingCompleted;
      double fileProgress;
      double epochProgress;
      double totalProgress;
      uint64_t currentEpoch;
      uint64_t currentFileFrameCount;
      uint64_t currentFileIndex;
      uint64_t currentFrame;
      uint64_t totalFiles;
      unsigned long eta;
      unsigned long timeTaken;
      std::string currentFileName;
    };

    enum class from_child_flag
    {
      train_batch_request_ack = 0x02,
      save_model_request_ack = 0x03,
    };

    enum class to_child_flag
    {
      train_batch_request = 0x02,
      save_model_request = 0x03,
    };

    void get_info(struct info &info);
    static void create_model(std::string dstDir);

    train_session(std::string metaFile, std::string trainingDataDir,
      uint64_t totalEpochs, uint64_t batchSize, uint64_t frameWidth,
      uint64_t frameHeight, bool suspendOnCompletion,
      Glib::Dispatcher &dispatcher);
    ~train_session();

  private:
    Glib::Dispatcher &dispatcher;
    bool suspendOnCompletion;
    uint64_t batchSize;
    uint64_t frameHeight;
    uint64_t frameWidth;
    uint64_t totalEpochs;
    info info;
    std::string metaFile;
    std::string trainingDataDir;
    std::atomic<bool> quit;
    std::mutex m;
    std::thread trainThread;

    void set_info(const struct info &info);
    static void train_thread_routine(train_session &trainSession);
  };
}

#endif
