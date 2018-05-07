#include "record_session.h"

static std::string create_record_name(std::string destinationFolder);

void ssbml::record_session::get_info(struct info &info)
{
  std::lock_guard<std::mutex> lock(m);
  info = this->info;
}

ssbml::record_session::record_session(Display *display, Window window,
  uint64_t frameWidth, uint64_t frameHeight, double fps,
  std::string gamepadDeviceFileName, std::string dstDir,
  Glib::Dispatcher &dispatcher) :
  dispatcher(dispatcher),
  fps(fps),
  frameHeight(frameHeight),
  frameWidth(frameWidth),
  quit(false),
  dstDir(dstDir),
  gamepadDeviceFileName(gamepadDeviceFileName),
  window(window),
  display(display),
  info({
    .recording = false,
    .totalTime = 0
  }),
  recordThread(record_thread_routine, std::ref(*this))
{

}

ssbml::record_session::~record_session()
{
  quit = true;
  recordThread.join();
}

void ssbml::record_session::set_info(const struct info &info)
{
  std::unique_lock<std::mutex> lock(m);
  this->info = info;
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
  bool recordButtonReleased = true;
  std::ofstream gamepadFile;
  struct info info =
  {
    .recording = false,
    .totalTime = 0
  };

  double microsecondsBetweenFrames = 1000000.0 / recordSession.fps;
  ssbml::timer t;

  while (!recordSession.quit)
  {
    if (info.recording)
    {
      info.totalTime = t.total_time();
      gamepadFile << gamepadListener;
      videoInput >> *vo;
    }

    if (gamepadListener.get_btn_mode())
    {
      if (recordButtonReleased)
      {
        recordButtonReleased = false;
        info.recording = !info.recording;
        if (info.recording)
        {
          info.filePrefix = create_record_name(recordSession.dstDir);
          vo = new ssbml::video_output(info.filePrefix + ".mp4",
            recordSession.frameWidth, recordSession.frameHeight,
            recordSession.fps);
          gamepadFile = std::ofstream((info.filePrefix + ".gamepad").c_str(),
            std::ofstream::binary);
          t.reset();
        }
        else
        {
          gamepadFile.close();
          delete vo;
          info.totalTime = 0;
          info.filePrefix = "";
        }
      }
    }
    else
    {
      recordButtonReleased = true;
    }

    gamepadListener >> info.c;
    recordSession.set_info(info);
    t.get_delta_time(microsecondsBetweenFrames);
  }
}
