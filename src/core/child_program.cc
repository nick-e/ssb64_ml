#include "child_program.h"

ssize_t ssbml::child_program::write_to(uint8_t flag)
{
  return write_to(&flag, 1);
}

ssize_t ssbml::child_program::write_to(uint8_t *buf, uint64_t length)
{
  ssize_t sent = write(writeFd, buf, length);
  if (sent < 0)
  {
    throw std::runtime_error(std::string("write: ") + strerror(errno));
  }
  return sent;
}

ssize_t ssbml::child_program::try_read_from(uint8_t *buf, uint64_t maxLength)
{
  ssize_t received = read(readFd, buf, maxLength);
  if (received < 0 && errno != EAGAIN)
  {
    throw std::runtime_error(std::string("read: ") + strerror(errno));
  }
  return received;
}

ssbml::child_program::child_program(std::string name, std::vector<char*> &args,
  bool waitForInit)
{
  int pipefd1[2], pipefd2[2];
  if (pipe(pipefd1) < 0)
  {
    throw std::runtime_error(std::string("pipe: ") + strerror(errno));
  }
  if (pipe(pipefd2) < 0)
  {
    close(pipefd1[0]);
    close(pipefd1[1]);
    throw std::runtime_error(std::string("pipe: ") + strerror(errno));
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
      throw std::runtime_error(std::string("dup2: ") + strerror(errno));
    }
    if (dup2(pipefd2[0], STDIN_FILENO) < 0)
    {
      close(pipefd1[1]);
      close(pipefd2[0]);
      throw std::runtime_error(std::string("dup2: ") + strerror(errno));
    }
    close(pipefd1[1]);
    close(pipefd2[0]);
    if (setvbuf(stdin, NULL, _IONBF, 0) < 0)
    {
      close(pipefd1[1]);
      close(pipefd2[0]);
      throw std::runtime_error(std::string("setvbuf: ") + strerror(errno));
    }
    if (execvp(name.c_str(), (char**)&args[0]) < 0)
    {
      close(pipefd1[1]);
      close(pipefd2[0]);
      throw std::runtime_error(std::string("execlp: ") + strerror(errno));
    }
  }
  close(pipefd1[1]);
  close(pipefd2[0]);
  readFd = pipefd1[0];
  writeFd = pipefd2[1];

  if (waitForInit)
  {
    uint8_t flag;
    try_read_from(&flag, 1);
    if (flag != (uint8_t)from_child_flag::child_initialized)
    {
      std::cerr << "Received unknown flag 0x" << std::setfill('0')
        << std::setw(2) << std::hex << (uint32_t)flag << std::endl;
    }
  }

  if (fcntl(readFd, F_SETFL, O_NONBLOCK) < 0)
  {
    close(pipefd1[0]);
    close(pipefd2[1]);
    throw std::runtime_error(std::string("fcntl: ") + strerror(errno));
  }
}

ssbml::child_program::~child_program()
{
  uint8_t flag;
  int opts;
  flag = (uint8_t)to_child_flag::shutdown_request;
  if (write(writeFd, &flag, sizeof(flag)) < 0)
  {
    perror("write");
  }
  else if ((opts = fcntl(readFd, F_GETFL)) < 0)
  {
    perror("fcntl");
  }
  else if (fcntl(readFd, F_SETFL, opts & (~O_NONBLOCK)) < 0)
  {
    perror("fcntl");
  }
  else if (read(readFd, &flag, sizeof(flag)) < 0)
  {
    perror("read");
  }
  if (flag != (uint8_t)from_child_flag::child_terminated)
  {
    std::cerr << "Received unknown flag 0x" << std::setfill('0')
      << std::setw(2) << std::hex << (uint32_t)flag << std::endl;
  }
  close(readFd);
  close(writeFd);
}
