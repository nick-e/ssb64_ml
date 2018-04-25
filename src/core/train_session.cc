#include "train_session.h"

void ssbml::train_session::set_train_info(bool modelLoaded,
  uint64_t currentEpoch, uint64_t currentFile, uint64_t currentFileFrameCount,
  std::string currentFileName, uint64_t currentFrame, double progress,
  uint64_t fileCount, bool doneTraining)
{
  std::unique_lock<std::mutex> lock(m);
  this->modelLoaded = modelLoaded;
  this->currentEpoch = currentEpoch;
  this->currentFile = currentFile;
  this->currentFileFrameCount = currentFileFrameCount;
  this->currentFileName = currentFileName;
  this->currentFrame = currentFrame;
  this->progress = progress;
  this->fileCount = fileCount;
  this->doneTraining = doneTraining;
  dispatcher.emit();
}

void ssbml::train_session::get_train_info(bool *modelLoaded,
  uint64_t *currentEpoch, uint64_t *currentFile,
  uint64_t *currentFileFrameCount, std::string &currentFileName,
  uint64_t *currentFrame, double *progress, uint64_t *fileCount,
  bool *doneTraining)
{
  std::lock_guard<std::mutex> lock(m);
  *modelLoaded = this->modelLoaded;
  *currentEpoch = this->currentEpoch;
  *currentFile = this->currentFile;
  *currentFileFrameCount = this->currentFileFrameCount;
  currentFileName = this->currentFileName;
  *currentFrame = this->currentFrame;
  *progress = this->progress;
  *fileCount = this->fileCount;
  *doneTraining = this->doneTraining;
}

bool ssbml::train_session::get_quit()
{
  return quit;
}

ssbml::train_session::~train_session()
{
  quit = true;
  trainThread.join();
}

void ssbml::train_session::create_model(std::string dstDir)
{
  char buf[1024];
  FILE *in = popen(("python ../src/python/create_model.py " + dstDir
    + " 256 144").c_str(), "r");
  size_t length;
  while ((length = fread(buf, 1, sizeof(buf), in)))
  {
    if (ferror(in))
    {
      perror("ssbml::train_session::create_model(): fread()");
      break;
    }
    if (length > 0)
    {
      std::cout << buf;
    }
    if (feof(in))
    {
      break;
    }
  }
  pclose(in);
}

bool has_suffix(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static int get_training_file_names(std::string trainingFilesDir,
  std::vector<std::string> &trainingFiles, uint64_t *totalFrames)
{
  uint64_t frames = 0;

  DIR *dir = opendir(trainingFilesDir.c_str());
  struct dirent *entry;
  if (dir == NULL)
  {
    perror("opendir");
    return -1;
  }
  while ((entry = readdir(dir)) != NULL)
  {
    std::string videoFileName = trainingFilesDir + "/" + entry->d_name;
    if (!has_suffix(videoFileName, ".mp4"))
    {
      continue;
    }
    struct stat s;
    if (stat(videoFileName.c_str(), &s) < 0)
    {
      perror("stat");
      closedir(dir);
      return - 1;
    }
    if ((s.st_mode & S_IFMT) != S_IFREG)
    {
      continue;
    }

    ssbml::video_file videoFile(videoFileName);
    trainingFiles.push_back(videoFileName.substr(0,
      videoFileName.find_last_of(".")));
    frames += videoFile.get_total_frames();
  }
  closedir(dir);

  *totalFrames = frames;
  return 0;
}

static void train_thread_routine(ssbml::train_session &trainSession,
  std::string modelDir, std::string trainingDataDir, uint64_t epochs,
  uint64_t batchSize, uint64_t frameWidth, uint64_t frameHeight)
{
  uint64_t totalFrames = 0, trainedFrames = 0, fileCount = 0;
  std::vector<std::string> trainingFiles;

  if (get_training_file_names(trainingDataDir, trainingFiles, &totalFrames) < 0)
  {
    return;
  }
  totalFrames *= epochs;
  fileCount = trainingFiles.size();
  trainSession.set_train_info(false, 0, 0, 0, "", 0, 0.0, 0, false);

  int readFd, writeFd;
  std::vector<char*> args;
  modelDir = modelDir.substr(0, modelDir.find_last_of("."));
  args.push_back(const_cast<char*>("python"));
  args.push_back(const_cast<char*>("../src/python/train.py"));
  args.push_back(const_cast<char*>(modelDir.c_str()));
  args.push_back(const_cast<char*>(std::to_string(frameWidth).c_str()));
  args.push_back(const_cast<char*>(std::to_string(frameHeight).c_str()));
  args.push_back(const_cast<char*>(std::to_string(batchSize).c_str()));
  args.push_back(NULL);
  if (ssbml::launch_program("python", (char**)&args[0], &readFd, &writeFd) < 0)
  {
    return;
  }

  ssbml::timer t;
  uint64_t rgbBufSize = frameWidth * frameHeight * 3,
    gamepadBufSize = sizeof(ssbml::gamepad::compressed),
    bufSize = rgbBufSize + gamepadBufSize * 2;
  uint8_t *buf = new uint8_t[bufSize], *rgbBuf = buf,
    *gamepadBuf = rgbBuf + rgbBufSize, *labelBuf = gamepadBuf + gamepadBufSize;
  bool modelLoaded = false;
  while (!modelLoaded)
  {
    if (trainSession.get_quit())
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
    trainSession.set_train_info(false, 0, 0, 0, "", 0, 0.0, 0, false);
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
        case (uint8_t)ssbml::train_session::Py2CC_Flag::ModelLoaded:
          modelLoaded = true;
          break;
        default:
          std::cerr << "Received close(readFd); flag 0x" << std::setfill('0')
            << std::setw(2) << std::hex << (uint32_t)flag << std::endl;
          break;
      }
    }
  }
  trainSession.set_train_info(true, 0, 0, 0, "", 0, 0.0, fileCount, false);

  for (uint64_t epoch = 0; epoch < epochs; ++epoch)
  {
    for (std::vector<std::string>::size_type file = 0; file
      < trainingFiles.size(); ++file)
    {
      std::string fileName = trainingFiles[file];
      ssbml::video_file videoFile(fileName + ".mp4");
      ssbml::gamepad_file gamepadFile(fileName + ".gamepad");
      uint64_t currentFileFrameCount = videoFile.get_total_frames();
      uint64_t batches = currentFileFrameCount / (batchSize + 1);
      uint64_t excessFrames = currentFileFrameCount - batches * batchSize;

      for (uint64_t batch = 0; batch < batches; ++batch)
      {
        trainSession.set_train_info(true, epoch, file, currentFileFrameCount,
          fileName, batch * batchSize, (double)trainedFrames / totalFrames,
          fileCount, false);

        buf[0] = (uint8_t)ssbml::train_session::CC2Py_Flag::Batch;
        if (write(writeFd, buf, 1) < 0)
        {
          close(readFd);
          close(writeFd);
          perror("write");
          delete[] buf;
          return;
        }
        // Send the frames and gamepad info in the batch
        for (uint64_t frame = 0; frame < batchSize; ++frame)
        {
          if (trainSession.get_quit())
          {
            close(readFd);
            close(writeFd);
            delete[] buf;
            return;
          }
          videoFile >> rgbBuf;
          gamepadFile >> gamepadBuf;
          gamepadFile >> labelBuf;
          gamepadFile.rewind();
          if (write(writeFd, buf, rgbBufSize + gamepadBufSize * 2) < 0)
          {
            close(readFd);
            close(writeFd);
            perror("write(2)");
            return;
          }
        }

        // Wait for response saying that the batch was trained
        t.get_delta_time();
        bool batchTrained = false;
        while (!batchTrained)
        {
          if (trainSession.get_quit())
          {
            close(readFd);
            close(writeFd);
            delete[] buf;
            return;
          }
          unsigned long deltaTime = t.get_delta_time();
          if (deltaTime < 33333)
          {
            usleep(33333 - deltaTime);
            t.get_delta_time();
          }
          ssize_t len = read(readFd, buf, bufSize);
          if (len < 0 && errno != EAGAIN)
          {
            close(readFd);
            close(writeFd);
            perror("read(2)");
            delete[] buf;
            return;
          }
          else if (len > 0)
          {
            uint8_t flag = buf[0];
            switch (flag)
            {
              case (uint8_t)ssbml::train_session::Py2CC_Flag::BatchTrained:
                batchTrained = true;
                break;
              default:
                std::cerr << "Received unknown flag 0x" << std::setfill('0')
                  << std::setw(2) << std::hex << (uint32_t)flag << std::endl;
                break;
            }
          }
        }
        trainedFrames += batchSize;
      }
      trainedFrames += excessFrames;
    }
  }

  trainSession.set_train_info(true, 0, 0, 0, "", 0, 1.0, fileCount, false);

  buf[0] = (uint8_t)ssbml::train_session::CC2Py_Flag::Done;
  if (write(writeFd, buf, 1) < 0)
  {
    close(readFd);
    close(writeFd);
    perror("write(3)");
    delete[] buf;
    return;
  }

  bool modelSaved = false;
  while (!modelSaved)
  {
    if (trainSession.get_quit())
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
      perror("read(3)");
      delete[] buf;
      return;
    }
    else if (len > 0)
    {
      uint8_t flag = buf[0];
      switch (flag)
      {
        case (uint8_t)ssbml::train_session::Py2CC_Flag::ModelSaved:
          modelSaved = true;
          break;
        default:
          std::cerr << "Received unknown flag 0x" << std::setfill('0')
            << std::setw(2) << std::hex << (uint32_t)flag << std::endl;
          close(readFd);
          close(writeFd);
          delete[] buf;
          return;
      }
    }
  }

  trainSession.set_train_info(true, 0, 0, 0, "", 0, 1.0, fileCount, true);
  close(readFd);
  close(writeFd);
  delete[] buf;
}

ssbml::train_session::train_session(std::string modelDir,
  std::string trainingDataDir, uint64_t epochs, uint64_t batchSize,
  uint64_t frameWidth, uint64_t frameHeight, Glib::Dispatcher &dispatcher) :
  dispatcher(dispatcher),
  quit(false),
  currentEpoch(0),
  currentFrame(0),
  currentFile(0),
  currentFileFrameCount(0),
  currentFileName(""),
  progress(0),
  fileCount(0),
  modelLoaded(false),
  doneTraining(false),
  trainThread(train_thread_routine, std::ref(*this), modelDir, trainingDataDir,
    epochs, batchSize, frameWidth, frameHeight)
{

}
