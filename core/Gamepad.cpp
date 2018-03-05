#include "Gamepad.h"

unsigned long SSBML::Gamepad::get_gamepad_device_file_names(std::string *dest)
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
