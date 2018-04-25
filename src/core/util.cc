#include "util.h"

int ssbml::launch_program(std::string file, char * const *args, int *readFd,
  int *writeFd)
{
  int pipefd1[2], pipefd2[2];
  if (pipe(pipefd1) < 0)
  {
    perror("pipe1");
    return -1;
  }
  if (pipe(pipefd2) < 0)
  {
    close(pipefd1[0]);
    close(pipefd1[1]);
    perror("pipe2");
    return -1;
  }
  pid_t pid = fork();
  if (pid == 0)
  {
    close(pipefd1[0]);
    close(pipefd2[1]);
    if (dup2(pipefd1[1], STDOUT_FILENO) < 0)
    {
      close(pipefd1[1]);
      close(pipefd2[0]);
      perror("dup2(1)");
      exit(EXIT_FAILURE);
    }
    if (dup2(pipefd2[0], STDIN_FILENO) < 0)
    {
      close(pipefd1[1]);
      close(pipefd2[0]);
      perror("dup2(1)");
      exit(EXIT_FAILURE);
    }
    close(pipefd1[1]);
    close(pipefd2[0]);
    if (setvbuf(stdin, NULL, _IONBF, 0) < 0)
    {
      close(pipefd1[1]);
      close(pipefd2[0]);
      perror("setvbuf");
      exit(EXIT_FAILURE);
    }
    if (execvp(file.c_str(), args) < 0)
    {
      close(pipefd1[1]);
      close(pipefd2[0]);
      perror("execlp");
      exit(EXIT_FAILURE);
    }
  }
  close(pipefd1[1]);
  close(pipefd2[0]);
  if (fcntl(pipefd1[0], F_SETFL, O_NONBLOCK) < 0)
  {
    close(pipefd1[0]);
    close(pipefd2[1]);
    perror("fcntl");
    return -1;
  }
  *readFd = pipefd1[0];
  *writeFd = pipefd2[1];
  return 0;
}

void ssbml::create_ppm(std::string path, uint8_t *rgbBuf,
  uint64_t width, uint64_t height)
{
  std::ofstream out(path);
  if (!out)
  {
    std::cerr << "Failed to open " << path << std::endl;
  }
  out << "P3\n" << width << " " << height << "\n255\n";
  for (uint64_t y = 0; y < height; ++y)
  {
    for (uint64_t x = 0; x < width; ++x)
    {
      uint64_t index = (y * width + x) * 3;
      out << std::to_string(rgbBuf[index]) << " "
        << std::to_string(rgbBuf[index + 1]) << " "
        << std::to_string(rgbBuf[index + 2]);
      if (x != 255)
      {
        out << " ";
      }
    }
    out << "\n";
  }
  out.close();
}

std::string ssbml::time_to_string(unsigned long time)
{
  unsigned long x = time / 100000;
  unsigned long dec = x % 10;
  x /= 10;
  unsigned long seconds = x % 60;
  x /= 60;
  unsigned long minutes = x % 60;
  x /= 60;
  unsigned long hourecordSession = x % 24;

  std::string str = "";
  if (hourecordSession < 10)
  {
    str += "0";
  }
  str += std::to_string(hourecordSession) + ":";
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
