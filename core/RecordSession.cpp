#include "RecordSession.h"

static void* pthread_routine(void *arg)
{
  bool recording = false, recordButtonReleased = true;
  SSBML::RecordSession *recordSession = (SSBML::RecordSession*)arg;
  recordSession->gamepad.init();
  while (!(recordSession->quit))
  {
    if (recordSession->gamepad.xbox)
    {
      if (recordButtonReleased)
      {
        recordButtonReleased = false;
        recording = !recording;
        if (recording)
        {
          std::cout << "started recording" << std::endl;
        }
        else
        {
          std::cout << "stopped recording" << std::endl;
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
  videoInput(display, window, 256, 144), destinationFolder(destinationFolder),
  quit(false)
{

}

void SSBML::RecordSession::init()
{
  if (pthread_create(&tid, NULL, pthread_routine, this) < 0)
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
