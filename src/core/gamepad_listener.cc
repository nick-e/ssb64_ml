#include "gamepad_listener.h"

bool ssbml::gamepad_listener::get_btn_mode()
{
  std::lock_guard<std::mutex> lock(m);
  return buttons.mode;
}

void ssbml::gamepad_listener::listen_thread_routine(ssbml::gamepad_listener &gl)
{
  struct input_event e;
  std::unique_lock<std::mutex> lock(gl.m);
  lock.unlock();
  while (!gl.quit)
  {
    if (read(gl.deviceFile, &e, sizeof(e)) < 0)
    {
      if (errno != EAGAIN)
      {
        perror("read_evdev(): read()");
      }
      continue;
    }
    if (e.type == EV_SYN)
    {
      continue;
    }

    lock.lock();
    switch (e.code)
    {
      case (int)BTN_A:
        gl.buttons.a = (bool)e.value;
        break;
      case (int)BTN_B:
        gl.buttons.b = (bool)e.value;
        break;
      case (int)BTN_X:
        gl.buttons.x = (bool)e.value;
        break;
      case (int)BTN_Y:
        gl.buttons.y = (bool)e.value;
        break;
      case (int)BTN_TR:
        gl.buttons.tr = (bool)e.value;
        break;
      case (int)BTN_TL:
        gl.buttons.tl = (bool)e.value;
        break;
      case (int)BTN_THUMBR:
        gl.buttons.thumbr = (bool)e.value;
        break;
      case (int)BTN_THUMBL:
        gl.buttons.thumbl = (bool)e.value;
        break;
      case (int)BTN_SELECT:
        gl.buttons.select = (bool)e.value;
        break;
      case (int)BTN_START:
        gl.buttons.start = (bool)e.value;
        break;
      case (int)BTN_MODE:
        gl.buttons.mode = (bool)e.value;
        break;
      case (int)ABS_X:
        gl.analogs.x = (int16_t)e.value;
        break;
      case (int)ABS_Y:
        gl.analogs.y = (int16_t)e.value;
        break;
      case (int)ABS_Z:
        gl.analogs.z = (int16_t)e.value;
        break;
      case (int)ABS_RX:
        gl.analogs.rx = (int16_t)e.value;
        break;
      case (int)ABS_RY:
        gl.analogs.ry = (int16_t)e.value;
        break;
      case (int)ABS_RZ:
        gl.analogs.rz = (int16_t)e.value;
        break;
      case (int)ABS_HAT0X:
        gl.analogs.hat0x = (int16_t)e.value;
        break;
      case (int)ABS_HAT0Y:
        gl.analogs.hat0y = (int16_t)e.value;
        break;
      default:
        std::cout << "Unkown event code: " << e.code <<
          std::endl << "type: " << e.type << std::endl << "value: " << e.value <<
          std::endl;
        break;
    }
    lock.unlock();
  }
}

ssbml::gamepad_listener::gamepad_listener(std::string deviceFileName) :
  threadsafe_gamepad(),
  quit(false),
  deviceFile(open(("/dev/input/" + deviceFileName).c_str(), O_RDONLY
    | O_NONBLOCK)),
  listenThread(listen_thread_routine, std::ref(*this))
{
  if (deviceFile < 0)
  {
    perror("SBML::gamepad_listener::gamepad_listener(): open()");
  }
  else
  {
    char name[1024];
    ioctl(deviceFile, EVIOCGNAME(sizeof(name)), name);
    deviceName = std::string(name);
  }
}

ssbml::gamepad_listener::~gamepad_listener()
{
  quit = true;
  listenThread.join();
  close(deviceFile);
}

std::string ssbml::gamepad_listener::get_device_name(std::string deviceFileName)
{
  int deviceFileFd = open(("/dev/input/" + deviceFileName).c_str(), O_RDONLY);
  if (deviceFileFd < 0)
  {
    perror("SBML::Gamepad::Gamepad(): open()");
    return "";
  }
  else
  {
    char deviceName[1024];
    ioctl(deviceFileFd, EVIOCGNAME(sizeof(deviceName)), deviceName);
    close(deviceFileFd);
    return std::string(deviceName);
  }
}

unsigned long ssbml::gamepad_listener::get_all_connected_gamepad_device_file_names(std::string *dest)
{
  struct dirent *dp;
  DIR *dfd;
  unsigned long count = 0;

  char *dir = (char*)"/dev/input";

  if ((dfd = opendir(dir)) == NULL)
  {
    perror("ssbml::Gamepad::get_gamepad_device_file_names(): opendir()");
    return 0;
  }

  while ((dp = readdir(dfd)) != NULL)
  {
    std::string fileName(dp->d_name);
    if (fileName.find("event") == 0 &&
      !access(("/dev/input/" + fileName).c_str(), R_OK))
    {
      dest[count] = fileName;
      count++;
    }
  }
  return count;
}
