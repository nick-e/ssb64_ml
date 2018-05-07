#include "test_session.h"

void ssbml::test_session::test_thread_routine(ssbml::test_session &testSession)
{
  char tmp1[2048];
  char tmp2[2048];
  char tmp3[2048];
  double microsecondsBetweenFrames = 1000000.0 / testSession.fps;
  ssize_t received;
  uint64_t rgbBufSize = testSession.frameWidth * testSession.frameHeight * 3;
  uint64_t gamepadBufSize = sizeof(ssbml::gamepad::compressed);
  uint64_t bufSize = 1 + rgbBufSize + gamepadBufSize;
  uint8_t *buf = new uint8_t[bufSize];
  uint8_t *rgbBuf = buf + 1;
  uint8_t *gamepadBuf = rgbBuf + rgbBufSize;
  std::vector<char*> args;
  std::string modelCkpt = testSession.modelMeta.substr(0,
    testSession.modelMeta.find_last_of("."));
  ssbml::video_input videoInput(testSession.display, testSession.window,
    testSession.frameWidth, testSession.frameHeight);
  ssbml::gamepad_listener gamepadListener(testSession.gamepadDeviceFileName);
  ssbml::timer timer;
  struct info info =
  {
    .testing = false,
    .modelLoaded = false
  };

  sprintf(tmp1, "%s", modelCkpt.c_str());
  sprintf(tmp2, "%s", std::to_string(testSession.frameWidth).c_str());
  sprintf(tmp3, "%s", std::to_string(testSession.frameHeight).c_str());
  args.push_back(const_cast<char*>("python"));
  args.push_back(const_cast<char*>("../src/python/test.py"));
  args.push_back(tmp1);
  args.push_back(tmp2);
  args.push_back(tmp3);
  args.push_back(NULL);
  child_program childProgram("python", args, false);

  while (!testSession.quit && !info.modelLoaded && (received
    = childProgram.try_read_from(buf, bufSize)) < 0)
  {
    timer.get_delta_time(66666);
    testSession.set_info(info);
  }
  if (received > 0)
  {
    if (buf[0] == (uint8_t)child_program::from_child_flag::child_initialized)
    {
        info.modelLoaded = true;
        testSession.set_info(info);
    }
    else
    {
      std::cerr << "Received unknown flag 0x" << std::setfill('0') << std::setw(2)
        << std::hex << (uint32_t)buf[0] << std::endl;
    }
  }

  ssbml::gamepad::compressed c;
  bool testButtonPressed = false;
  while (!testSession.quit)
  {
    timer.get_delta_time(microsecondsBetweenFrames);
    if (gamepadListener.get_btn_mode())
    {
      if (!testButtonPressed)
      {
        info.testing = !info.testing;
        testButtonPressed = true;
      }
    }
    else
    {
      testButtonPressed = false;
    }

    if (info.testing)
    {
      buf[0] = (uint8_t)ssbml::test_session::to_child_flag::test_batch_request;
      testSession.gamepadSpoofer.compress(c);
      videoInput >> rgbBuf;
      memcpy(gamepadBuf, &c, sizeof(c));
      childProgram.write_to(buf, bufSize);

      while (!testSession.quit && (received = childProgram.try_read_from(buf,
        bufSize)) < 0)
        ;
      if (received > 0)
      {
        if (buf[0] == (uint8_t)from_child_flag::test_batch_request_ack)
        {
          memcpy(&info.c, buf + 1, sizeof(c));
          testSession.gamepadSpoofer << info.c;
        }
        else
        {
          std::cerr << "Received unknown flag 0x" << std::setfill('0')
            << std::setw(2) << std::hex << (uint32_t)buf[0] << std::endl;
        }
      }
    }
    else
    {
      testSession.gamepadSpoofer << gamepadListener;
      gamepadListener >> info.c;
    }
    testSession.set_info(info);
  }

  delete[] buf;
}

void ssbml::test_session::set_info(const struct info &info)
{
  std::unique_lock<std::mutex> lock(m);
  this->info = info;
  dispatcher.emit();
}

void ssbml::test_session::get_info(struct info &info)
{
  std::lock_guard<std::mutex> lock(m);
  info = this->info;
}

ssbml::test_session::test_session(Display *display, Window window,
  uint64_t frameWidth, uint64_t frameHeight, double fps,
  std::string gamepadDeviceFileName, std::string modelMeta,
  gamepad_spoofer &gamepadSpoofer, Glib::Dispatcher &dispatcher) :
  dispatcher(dispatcher),
  fps(fps),
  frameHeight(frameHeight),
  frameWidth(frameWidth),
  quit(false),
  gamepadDeviceFileName(gamepadDeviceFileName),
  modelMeta(modelMeta),
  gamepadSpoofer(gamepadSpoofer),
  display(display),
  window(window),
  info({
    .testing = false,
    .modelLoaded = false
  }),
  testThread(test_thread_routine, std::ref(*this))
{

}

ssbml::test_session::~test_session()
{
  quit = true;
  testThread.join();
}
