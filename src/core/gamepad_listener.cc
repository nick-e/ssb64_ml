#include "gamepad_listener.h"

bool ssbml::gamepad_listener::get_btn_mode()
{
  std::lock_guard<std::mutex> lock(m);
  return buttons.mode;
}

int ssbml::gamepad_listener::get_device_file()
{
  return deviceFile;
}

bool ssbml::gamepad_listener::get_quit()
{
  return quit;
}

void ssbml::gamepad_listener::decompress(const compressed &c)
{
  std::lock_guard<std::mutex> lock(m);
  buttons.a = c.buttons & 0x1;
  buttons.b = (c.buttons >> 1) & 0x1;
  buttons.x = (c.buttons >> 2) & 0x1;
  buttons.y = (c.buttons >> 3) & 0x1;
  buttons.tr = (c.buttons >> 4) & 0x1;
  buttons.tl = (c.buttons >> 5) & 0x1;
  buttons.thumbr = (c.buttons >> 6) & 0x1;
  buttons.thumbl = (c.buttons >> 7) & 0x1;
  buttons.select = (c.buttons >> 8) & 0x1;
  buttons.start = (c.buttons >> 9) & 0x1;
  buttons.mode = (c.buttons >> 10) & 0x1;
  analogs = c.analogs;
}

void ssbml::gamepad_listener::compress(compressed &c)
{
  std::lock_guard<std::mutex> lock(m);
  c.buttons = buttons.a | (buttons.b << 1) | (buttons.x << 2)
    | (buttons.y << 3) | (buttons.tr << 4) | (buttons.tl << 5)
    | (buttons.thumbr << 6) | (buttons.thumbl << 7) | (buttons.select << 8)
    | (buttons.start << 9) | (buttons.mode << 10);
  c.analogs = analogs;
}

std::ofstream& operator<<(std::ofstream &stream, ssbml::gamepad_listener &g)
{
  ssbml::gamepad::compressed c;
  g.compress(c);
  stream.write((char*)&c, sizeof(c));
  return stream;
}


static void listen_thread_routine(ssbml::gamepad_listener &gl)
{
  int deviceFile = gl.get_device_file();
  struct input_event e;
  std::unique_lock<std::mutex> lock(gl.m);
  lock.unlock();
  while (!gl.get_quit())
  {
    if (read(deviceFile, &e, sizeof(e)) < 0)
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
  quit(false),
  deviceFileName(deviceFileName),
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
    deviceFileName = std::string(name);
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
