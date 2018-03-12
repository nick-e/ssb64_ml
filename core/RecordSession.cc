#include "RecordSession.h"

// time is in microseconds
static std::string time_to_string(unsigned long time)
{
  unsigned long x = time / 100000;
  unsigned long dec = x % 10;
  x /= 10;
  unsigned long seconds = x % 60;
  x /= 60;
  unsigned long minutes = x % 60;
  x /= 60;
  unsigned long hours = x % 24;

  std::string str = "";
  if (hours < 10)
  {
    str += "0";
  }
  str += std::to_string(hours) + ":";
  if (minutes < 10)
  {
    str += "0";
  }
  str += std::to_string(minutes) + ":";
  if (seconds < 10)
  {
    str += "0";
  }
  str += std::to_string(seconds) + "." + std::to_string(dec);

  return str;
}

static std::string create_record_name(std::string destinationFolder)
{
  char str[1024];
  time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  strftime(str, 1024, "%Y-%m-%d_%H-%M-%S", localtime(&now));
  return destinationFolder + std::string(str);
}

static void* record_routine(void *arg)
{
  cv::VideoWriter videoWriter;
  cv::Size size = cv::Size(FRAME_WIDTH, FRAME_HEIGHT);
  unsigned char frame[FRAME_WIDTH * FRAME_HEIGHT * 3];
  int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
  double fps = 30;
  double microsecondsBetweenFrames = 1000000.0 / fps, timeSinceLastRecord = 0.0;
  char gamepadBinary[17];
  int gamepadOutputFd;
  std::string filePrefix;
  unsigned long deltaTime, totalTime = 0;
  std::chrono::high_resolution_clock::time_point now =
    std::chrono::high_resolution_clock::now(), previousNow =
    std::chrono::high_resolution_clock::now();
  bool recording = false, recordButtonReleased = true;
  SSBML::RecordSession *recordSession = (SSBML::RecordSession*)arg;
  recordSession->gamepad.init();
  while (!(recordSession->quit))
  {
    previousNow = now;
    now = std::chrono::high_resolution_clock::now();
    deltaTime = (std::chrono::duration_cast<std::chrono::microseconds>(now -
      previousNow)).count();

    if (recording)
    {
      totalTime += deltaTime;
      timeSinceLastRecord += deltaTime;
      while (timeSinceLastRecord >= microsecondsBetweenFrames)
      {
        timeSinceLastRecord -= microsecondsBetweenFrames;
        recordSession->gamepad.get_binary(gamepadBinary);
        recordSession->videoInput.get_frame(frame);
        if (write(gamepadOutputFd, gamepadBinary, sizeof(gamepadBinary)) < 0)
        {
          perror("");
        }
        cv::Mat frameMat(FRAME_HEIGHT, FRAME_WIDTH, CV_8UC3, frame);
        videoWriter.write(frameMat);
      }
      if (deltaTime < 1000)
      {
        usleep(3000);
      }
    }

    if (recordSession->gamepad.xbox)
    {
      if (recordButtonReleased)
      {
        recordButtonReleased = false;
        recording = !recording;
        if (recording)
        {
          std::cout << "started recording" << std::endl;
          filePrefix = create_record_name(recordSession->destinationFolder);
          std::cout << filePrefix << std::endl;
          videoWriter.open(filePrefix + ".avi", fourcc, fps, size);
          if ((gamepadOutputFd = open((filePrefix + ".gamepad").c_str(), O_WRONLY | O_TRUNC | O_CREAT)) < 0)
          {

            perror("record_routine(): open()");
            recording = false;
            continue;
          }
          totalTime = 0;
          timeSinceLastRecord = microsecondsBetweenFrames;
        }
        else
        {
          std::cout << "stopped recording" << std::endl;
          close(gamepadOutputFd);
          videoWriter.release();
        }
      }
    }
    else
    {
      recordButtonReleased = true;
    }
  }
  recordSession->gamepad.end();
  return NULL;
}

SSBML::RecordSession::RecordSession(Display *display, Window window,
  std::string gamepadDeviceFileName, std::string destinationFolder) :
  display(display), window(window), gamepad(gamepadDeviceFileName),
  videoInput(display, window, FRAME_WIDTH, FRAME_HEIGHT), destinationFolder(destinationFolder),
  quit(false)
{

}

void SSBML::RecordSession::init()
{
  if (pthread_create(&tid, NULL, record_routine, this) < 0)
  {
    perror("SSBML::RecordWindow::RecordWindow(): pthread_create()");
  }
}

void SSBML::RecordSession::end()
{
  quit = true;
  if (pthread_join(tid, NULL) < 0)
  {
    perror("SSBML::RecordWindow::RecordWindow(): pthread_join()");
  }
}
