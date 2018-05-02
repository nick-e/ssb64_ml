#include "record_session.h"

static std::string create_record_name(std::string destinationFolder);

void ssbml::record_session::get_record_info(bool *recording,
  unsigned long *totalTime, std::string &filePrefix)
{
  std::lock_guard<std::mutex> lock(m);
  *recording = this->recording;
  *totalTime = this->totalTime;
  filePrefix = this->filePrefix;
}

ssbml::record_session::record_session(Display *display, Window window,
  uint64_t frameWidth, uint64_t frameHeight, double fps,
  std::string gamepadDeviceFileName, std::string dstDir,
  Glib::Dispatcher &dispatcher) :
  dispatcher(dispatcher),
  recording(false),
  fps(fps),
  frameHeight(frameHeight),
  frameWidth(frameWidth),
  totalTime(0),
  quit(false),
  dstDir(dstDir),
  filePrefix(),
  gamepadDeviceFileName(gamepadDeviceFileName),
  window(window),
  display(display),
  recordThread(record_thread_routine, std::ref(*this))
{

}

ssbml::record_session::~record_session()
{
  quit = true;
  recordThread.join();
}

void ssbml::record_session::set_record_info(bool recording,
  unsigned long totalTime, std::string filePrefix)
{
  std::unique_lock<std::mutex> lock(m);
  this->recording = recording;
  this->totalTime = totalTime;
  this->filePrefix = filePrefix;
  dispatcher.emit();
}

static std::string create_record_name(std::string destinationFolder)
{
  char str[1024];
  time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  strftime(str, 1024, "%Y-%m-%d_%H-%M-%S", localtime(&now));
  return destinationFolder + std::string(str);
}

void ssbml::record_session::record_thread_routine(
  ssbml::record_session &recordSession)
{
  ssbml::video_input videoInput(recordSession.display, recordSession.window,
    recordSession.frameWidth, recordSession.frameHeight);
  ssbml::gamepad_listener gamepadListener(recordSession.gamepadDeviceFileName);
  ssbml::video_output *vo = nullptr;
  bool recording = false;
  bool recordButtonReleased = true;
  std::ofstream gamepadFile;
  std::string filePrefix;

  double microsecondsBetweenFrames = 1000000.0 / recordSession.fps;
  ssbml::timer t;

  while (!recordSession.quit)
  {
    if (recording)
    {
      recordSession.set_record_info(recording, t.total_time(), filePrefix);
      gamepadFile << gamepadListener;
      videoInput >> *vo;
    }

    if (gamepadListener.get_btn_mode())
    {
      if (recordButtonReleased)
      {
        recordButtonReleased = false;
        recording = !recording;
        if (recording)
        {
          filePrefix = create_record_name(recordSession.dstDir);
          vo = new ssbml::video_output(filePrefix + ".mp4",
            recordSession.frameWidth, recordSession.frameHeight,
            recordSession.fps);
          gamepadFile = std::ofstream((filePrefix + ".gamepad").c_str(),
            std::ofstream::binary);
          t.reset();
        }
        else
        {
          gamepadFile.close();
          delete vo;
          recordSession.set_record_info(recording, 0, "");
        }
      }
    }
    else
    {
      recordButtonReleased = true;
    }

    unsigned long deltaTime = t.get_delta_time();
    if (deltaTime < microsecondsBetweenFrames)
    {
      usleep(microsecondsBetweenFrames - deltaTime);
      deltaTime += t.get_delta_time();
    }
  }
}
