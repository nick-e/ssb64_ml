#include "test_session.h"

static void test_thread_routine(ssbml::test_session &testSession,
  Display *display, Window window, uint64_t frameWidth, uint64_t frameHeight,
  double fps, std::string gamepadDeviceFileName, std::string modelMeta,
  ssbml::gamepad_spoofer &gamepadSpoofer)
{
  ssbml::video_input videoInput(display, window, frameWidth, frameHeight);
  ssbml::gamepad_listener gamepadListener(gamepadDeviceFileName);
  ssbml::timer t;

  int readFd, writeFd;
  std::vector<char*> args;
  std::string modelCkpt = modelMeta.substr(0, modelMeta.find_last_of("."));
  args.push_back(const_cast<char*>("python"));
  args.push_back(const_cast<char*>("../src/python/test.py"));
  args.push_back(const_cast<char*>(modelCkpt.c_str()));
  args.push_back(const_cast<char*>(std::to_string(frameWidth).c_str()));
  args.push_back(const_cast<char*>(std::to_string(frameHeight).c_str()));
  args.push_back(NULL);
  if (ssbml::launch_program("python", (char**)&args[0], &readFd, &writeFd) < 0)
  {
    return;
  }

  uint64_t rgbBufSize = frameWidth * frameHeight * 3,
    gamepadBufSize = sizeof(ssbml::gamepad::compressed),
    bufSize = 1 + rgbBufSize + gamepadBufSize;
  uint8_t *buf = new uint8_t[bufSize], *rgbBuf = buf + 1,
    *gamepadBuf = rgbBuf + rgbBufSize;
  bool modelLoaded = false;
  while (!modelLoaded)
  {
    if (testSession.get_quit())
    {
      close(readFd);
      close(writeFd);
      delete[] buf;
      return;
    }
    unsigned long deltaTime = t.get_delta_time();
    if (deltaTime < 66666)
    {
      usleep(66666 - deltaTime);
      t.get_delta_time();
    }
    ssize_t len = read(readFd, buf, bufSize);
    if (len < 0 && errno != EAGAIN)
    {
      close(readFd);
      close(writeFd);
      perror("read");
      delete[] buf;
      return;
    }
    if (len > 0)
    {
      uint8_t flag = buf[0];
      switch (flag)
      {
        case (uint8_t)ssbml::test_session::Py2CC_Flag::ModelLoaded:
          modelLoaded = true;
          break;
        default:
          std::cerr << "Received close(readFd); flag 0x" << std::setfill('0')
            << std::setw(2) << std::hex << (uint32_t)flag << std::endl;
          break;
      }
    }
  }
  testSession.set_test_info(false, true);

  ssbml::gamepad::compressed c;
  bool testButtonPressed = false;
  bool testing = false;
  while (!testSession.get_quit())
  {
    unsigned long deltaTime = t.get_delta_time();
    if (deltaTime < 33333)
    {
      usleep(33333 - deltaTime);
      t.get_delta_time();
    }

    if (gamepadListener.get_btn_mode())
    {
      if (!testButtonPressed)
      {
        testing = !testing;
        testSession.set_test_info(testing, true);
        testButtonPressed = true;
      }
    }
    else
    {
      testButtonPressed = false;
    }

    if (testing)
    {
      buf[0] = (uint8_t)ssbml::test_session::CC2Py_Flag::Data;
      videoInput >> rgbBuf;
      memcpy(gamepadBuf, &c, sizeof(c));

      if (write(writeFd, buf, bufSize) < 0)
      {
        close(readFd);
        close(writeFd);
        perror("write");
        delete[] buf;
        return;
      }

      bool receivedOutput = false;
      while (!testSession.get_quit() && !receivedOutput)
      {
        ssize_t len = read(readFd, buf, bufSize);
        if (len < 0 && errno != EAGAIN)
        {
          close(readFd);
          close(writeFd);
          perror("read(2)");
          delete[] buf;
          return;
        }
        if (len > 0)
        {
          uint8_t flag = buf[0];
          switch (flag)
          {
            case (uint8_t)ssbml::test_session::Py2CC_Flag::Output:
              receivedOutput = true;
              memcpy(&c, buf + 1, sizeof(c));
              gamepadSpoofer << c;
              break;
            default:
              std::cerr << "Received close(readFd); flag 0x" << std::setfill('0')
                << std::setw(2) << std::hex << (uint32_t)flag << std::endl;
              break;
          }
        }
      }
    }
  }

  buf[0] = (uint8_t)ssbml::test_session::CC2Py_Flag::Done;
  if (write(writeFd, buf, 1) < 0)
  {
    close(readFd);
    close(writeFd);
    perror("write(3)");
    delete[] buf;
    return;
  }

  close(readFd);
  close(writeFd);
  delete[] buf;
}

void ssbml::test_session::set_test_info(bool testing, bool modelLoaded)
{
  std::unique_lock<std::mutex> lock(m);
  this->testing = testing;
  this->modelLoaded = modelLoaded;
  dispatcher.emit();
}

void ssbml::test_session::get_test_info(bool *testing, bool *modelLoaded)
{
  std::lock_guard<std::mutex> lock(m);
  *testing = this->testing;
  *modelLoaded = this->modelLoaded;
}

ssbml::test_session::test_session(Display *display, Window window,
  uint64_t frameWidth, uint64_t frameHeight, double fps,
  std::string gamepadDeviceFileName, std::string modelMeta,
  gamepad_spoofer &gamepadSpoofer, Glib::Dispatcher &dispatcher) :
  dispatcher(dispatcher),
  quit(false),
  testing(false),
  testThread(test_thread_routine, std::ref(*this), display, window, frameWidth,
    frameHeight, fps, gamepadDeviceFileName, modelMeta,
    std::ref(gamepadSpoofer))
{

}

ssbml::test_session::~test_session()
{
  quit = true;
  testThread.join();
}

bool ssbml::test_session::get_quit()
{
  return quit;
}
