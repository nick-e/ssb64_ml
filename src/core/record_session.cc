#include "record_session.h"

void ssbml::record_session::set_record_info(bool recording,
  unsigned long totalTime, std::string filePrefix)
{
  std::unique_lock<std::mutex> lock(m);
  this->recording = recording;
  this->totalTime = totalTime;
  this->filePrefix = filePrefix;
  dispatcher.emit();
}

void ssbml::record_session::get_record_info(bool *recording,
  unsigned long *totalTime, std::string &filePrefix)
{
  std::lock_guard<std::mutex> lock(m);
  *recording = this->recording;
  *totalTime = this->totalTime;
  filePrefix = this->filePrefix;
}

static std::string create_record_name(std::string destinationFolder)
{
  char str[1024];
  time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  strftime(str, 1024, "%Y-%m-%d_%H-%M-%S", localtime(&now));
  return destinationFolder + std::string(str);
}

static void record_thread_routine(ssbml::record_session &recordSession,
  Display *display, Window window, uint64_t frameWidth, uint64_t frameHeight,
  double fps, std::string gamepadDeviceFileName, std::string dstDir)
{
  ssbml::video_input videoInput(display, window, frameWidth, frameHeight);
  ssbml::gamepad_listener gamepadListener(gamepadDeviceFileName);
  ssbml::video_output *vo = nullptr;
  bool recording = false;
  bool recordButtonReleased = true;
  std::ofstream gamepadFile;
  std::string filePrefix;

  double microsecondsBetweenFrames = 1000000.0 / fps;
  ssbml::timer t, t2;

  while (!(recordSession.get_quit()))
  {
    if (recording)
    {
      recordSession.set_record_info(recording, t.total_time(), filePrefix);

      gamepadFile << gamepadListener;

      unsigned long deltaTime2 = t2.get_delta_time();
      videoInput >> *vo;
      //std::cout << "\t" << t2.get_delta_time() << std::endl;
      if ((deltaTime2 = t2.get_delta_time()) > microsecondsBetweenFrames)
      {
        //std::cerr << "Getting frame took " << deltaTime2 << " microseconds" << std::endl;
      }

      //std::cout << "\t" << t2.get_delta_time() << std::endl;
      if ((deltaTime2 = t2.get_delta_time()) > microsecondsBetweenFrames)
      {
        //std::cerr << "Saving frame took " << deltaTime2 << " microseconds" << std::endl;
      }
    }

    if (gamepadListener.get_btn_mode())
    {
      if (recordButtonReleased)
      {
        recordButtonReleased = false;
        recording = !recording;
        if (recording)
        {
          filePrefix = create_record_name(dstDir);
          vo = new ssbml::video_output(filePrefix + ".mp4", frameWidth,
            frameHeight, fps);
          gamepadFile = std::ofstream((filePrefix + ".gamepad").c_str(), std::ofstream::binary);
          t.reset();
        }
        // done recording a video
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
    //std::cout << deltaTime << std::endl;
  }
}

ssbml::record_session::record_session(Display *display, Window window,
  uint64_t frameWidth, uint64_t frameHeight, double fps,
  std::string gamepadDeviceFileName, std::string dstDir,
  Glib::Dispatcher &dispatcher) :
  dispatcher(dispatcher),
  quit(false),
  recording(false),
  totalTime(0),
  filePrefix(""),
  recordThread(record_thread_routine, std::ref(*this), display, window,
    frameWidth, frameHeight, fps, gamepadDeviceFileName, dstDir)
{

}

ssbml::record_session::~record_session()
{
  quit = true;
  recordThread.join();
}

bool ssbml::record_session::get_quit()
{
  return quit;
}
