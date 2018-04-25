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

#include <gtkmm-3.0/gtkmm.h>

#include "video_file.h"
#include "timer.h"
#include "util.h"
#include "gamepad_file.h"

namespace ssbml
{
  class train_session
  {
  public:
    enum class Py2CC_Flag
    {
      ModelLoaded = 0x41,
      BatchTrained = 0x17,
      ModelSaved = 0x99
    };

    enum class CC2Py_Flag
    {
      Batch = 0x33,
      Done = 0x80
    };

    train_session(std::string modelDir, std::string trainingDataDir,
      uint64_t epochs, uint64_t batchSize, uint64_t frameWidth,
      uint64_t frameHeight, Glib::Dispatcher &dispatcher);
    ~train_session();

    void set_train_info(bool modelLoaded, uint64_t currentEpoch,
      uint64_t currentFile, uint64_t currentFileFrameCount,
      std::string currentFileName, uint64_t currentFrame, double progress,
      uint64_t fileCount, bool doneTraining);
    void get_train_info(bool *modelLoaded, uint64_t *currentEpoch,
      uint64_t *currentFile, uint64_t *currentFileFrameCount,
      std::string &currentFileName, uint64_t *currentFrame, double *progress,
      uint64_t *fileCount, bool *doneTraining);
    bool get_quit();

    static void create_model(std::string dstDir);

  private:
    Glib::Dispatcher &dispatcher;
    std::atomic<bool> quit;
    uint64_t currentEpoch;
    uint64_t currentFrame;
    uint64_t currentFile;
    uint64_t currentFileFrameCount;
    std::string currentFileName;
    double progress;
    uint64_t fileCount;
    bool modelLoaded;
    bool doneTraining;
    std::mutex m;
    std::thread trainThread;
  };
}

#endif
