#include "Gamepad.h"

int SSBML::Gamepad::get_device_file_fd()
{
  return deviceFileFd;
}

static void read_evdev(SSBML::Gamepad *gamepad)
{
  int len, deviceFileFd = gamepad->get_device_file_fd();
  struct input_event e;
  if ((len = read(deviceFileFd, &e, sizeof(e))) < 0)
  {
    if (errno != EAGAIN)
    {
      perror("read_evdev(): read()");
    }
    return;
  }
  if (e.type == EV_SYN)
  {
    return;
  }
  switch (e.code)
  {
    case (int)SSBML::Gamepad::EventCode::A:
      gamepad->A = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::B:
      gamepad->B = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::X:
      gamepad->X = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::Y:
      gamepad->Y = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::RB:
      gamepad->RB = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::LB:
      gamepad->LB = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::Select:
      gamepad->select = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::Start:
      gamepad->start = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::Xbox:
      gamepad->xbox = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::RT:
      gamepad->RT = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::LT:
      gamepad->LT = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::DPadY:
      gamepad->dpadY = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::DPadX:
      gamepad->dpadX = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::LAnalogX:
      gamepad->lanalogX = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::LAnalogY:
      gamepad->lanalogY = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::RAnalogX:
      gamepad->ranalogX = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::RAnalogY:
      gamepad->ranalogY = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::LToggle:
      gamepad->ltoggle = e.value;
      break;
    case (int)SSBML::Gamepad::EventCode::RToggle:
      gamepad->rtoggle = e.value;
      break;
    default:
      std::cout << "Unkown xbox event:" << std::endl << "\tcode: " << e.code <<
        std::endl << "type: " << e.type << std::endl << "value: " << e.value <<
        std::endl;
      break;
  }
  //std::cout << e.type << ", " << e.code << ", " << ((int)e.value) << std::endl;
  return;
}

static void* pthread_routine(void *arg)
{
  SSBML::Gamepad *gamepad = (SSBML::Gamepad*)arg;
  while (!gamepad->quit)
  {
    read_evdev(gamepad);
  }
  return NULL;
}

int SSBML::Gamepad::end()
{
  quit = true;
  if (pthread_join(tid, NULL) < 0)
  {
    perror("Gamepad::end(): pthread_join()");
    return -1;
  }
  close(deviceFileFd);
  std::cout << "Gamepad ended." << std::endl;
  return 0;
}

int SSBML::Gamepad::init()
{
  if ((deviceFileFd = open(deviceFilePath.c_str(), O_RDONLY | O_NONBLOCK)) < 0)
  {
    perror("Gamepad::init(): open()");
    return -1;
  }

  A = false;
  B = false;
  X = false;
  Y = false;
  RB = false;
  LB = false;
  select = false;
  start = false;
  xbox = false;
  ltoggle = false;
  rtoggle = false;
  RT = 0;
  LT = 0;
  dpadY = 0;
  dpadX = 0;
  lanalogX = 0;
  lanalogY = 0;
  ranalogX = 0;
  ranalogY = 0;

  if (pthread_create(&tid, NULL, pthread_routine, this) < 0)
  {
    perror("Gamepad::init(): pthread_create()");
    return -1;
  }
  std::cout << "Gamepad initiated." << std::endl;
  return 0;
}

SSBML::Gamepad::Gamepad(std::string deviceFileName) : A(false), B(false),
  X(false), Y(false), RB(false), LB(false), select(false), start(false),
  xbox(false), ltoggle(false), rtoggle(false), quit(false), RT(0), LT(0),
  dpadY(0), dpadX(0), lanalogX(0), lanalogY(0), ranalogX(0), ranalogY(0),
  deviceFileName(deviceFileName), deviceFilePath("/dev/input/" + deviceFileName)
{

  deviceFileFd = open(("/dev/input/" + deviceFileName).c_str(), O_RDONLY);
  if (deviceFileFd < 0)
  {
    perror("SBML::Gamepad::Gamepad(): open()");
  }
  else
  {
    char name[1024];
    ioctl(deviceFileFd, EVIOCGNAME(sizeof(name)), name);
    close(deviceFileFd);
    deviceFileName = std::string(name);
  }
}

std::string SSBML::Gamepad::get_device_name(std::string deviceFileName)
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

unsigned long SSBML::Gamepad::get_all_connected_gamepad_device_file_names(std::string *dest)
{
  struct dirent *dp;
  DIR *dfd;
  unsigned long count = 0;

  char *dir = (char*)"/dev/input";

  if ((dfd = opendir(dir)) == NULL)
  {
    perror("SSBML::Gamepad::get_gamepad_device_file_names(): opendir()");
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
