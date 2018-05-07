#include "train_session.h"

void ssbml::train_session::set_info(const struct info &info)
{
  std::unique_lock<std::mutex> lock(m);
  this->info = info;
  dispatcher.emit();
}

void ssbml::train_session::get_info(struct info &info)
{
  std::lock_guard<std::mutex> lock(m);
  info = this->info;
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
    perror(trainingFilesDir.c_str());
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

ssbml::train_session::train_session(std::string metaFile,
  std::string trainingDataDir, uint64_t totalEpochs, uint64_t batchSize,
  uint64_t frameWidth, uint64_t frameHeight, bool suspendOnCompletion,
  Glib::Dispatcher &dispatcher) :
  dispatcher(dispatcher),
  suspendOnCompletion(suspendOnCompletion),
  batchSize(batchSize),
  frameHeight(frameHeight),
  frameWidth(frameWidth),
  totalEpochs(totalEpochs),
  metaFile(metaFile),
  trainingDataDir(trainingDataDir),
  quit(false),
  trainThread(train_thread_routine, std::ref(*this))
{

}

ssbml::train_session::~train_session()
{
  quit = true;
  trainThread.join();
}

void ssbml::train_session::train_thread_routine(
  ssbml::train_session &trainSession)
{
  char tmp1[2048];
  char tmp2[2048];
  char tmp3[2048];
  char tmp4[2048];
  ssize_t received;
  uint8_t *buf;
  uint8_t *gamepadBuf;
  uint8_t *labelBuf;
  uint8_t *rgbBuf;
  uint64_t gamepadBufSize = sizeof(ssbml::gamepad::compressed);
  uint64_t rgbBufSize = trainSession.frameWidth * trainSession.frameHeight * 3;
  uint64_t bufSize = rgbBufSize + gamepadBufSize * 2;
  uint64_t totalFrames = 0;
  uint64_t totalFramesPerEpoch = 0;
  uint64_t trainedFrames = 0;
  std::string modelName = trainSession.metaFile.substr(0,
    trainSession.metaFile.find_last_of("."));
  std::vector<std::string> trainingFiles;
  std::vector<char*> args;
  timer timer;
  struct info info =
  {
    .modelLoaded = false,
    .trainingCompleted = false,
    .fileProgress = 0.0,
    .epochProgress = 0.0,
    .totalProgress = 0.0,
    .currentEpoch = 0,
    .currentFileFrameCount = 0,
    .currentFileIndex = 0,
    .currentFrame = 0,
    .totalFiles = 0,
    .eta = 0,
    .timeTaken = 0
  };

  trainSession.set_info(info);
  if (get_training_file_names(trainSession.trainingDataDir, trainingFiles,
    &totalFramesPerEpoch) < 0)
  {
    return;
  }
  totalFrames = totalFramesPerEpoch * trainSession.totalEpochs;
  info.totalFiles = trainingFiles.size();

  sprintf(tmp1, "%s", modelName.c_str());
  sprintf(tmp2, "%s", std::to_string(trainSession.frameWidth).c_str());
  sprintf(tmp3, "%s", std::to_string(trainSession.frameHeight).c_str());
  sprintf(tmp4, "%s", std::to_string(trainSession.batchSize).c_str());
  args.push_back(const_cast<char*>("python"));
  args.push_back(const_cast<char*>("../src/python/train.py"));
  args.push_back(tmp1);
  args.push_back(tmp2);
  args.push_back(tmp3);
  args.push_back(tmp4);
  args.push_back(NULL);
  child_program childProgram("python", args, false);

  buf = new uint8_t[bufSize];
  rgbBuf = buf;
  gamepadBuf = rgbBuf + rgbBufSize;
  labelBuf = gamepadBuf + gamepadBufSize;
  while (!trainSession.quit && (received = childProgram.try_read_from(buf,
    bufSize)) < 0)
  {
    timer.get_delta_time(66666);
    trainSession.set_info(info);
  }
  if (received > 0)
  {
    if (buf[0] == (uint8_t)child_program::from_child_flag::child_initialized)
    {
        info.modelLoaded = true;
        trainSession.set_info(info);
    }
    else
    {
      std::cerr << "Received unknown flag 0x" << std::setfill('0') << std::setw(2)
        << std::hex << (uint32_t)buf[0] << std::endl;
    }
  }

  timer.reset();
  for (uint64_t currentEpoch = 0; currentEpoch < trainSession.totalEpochs
    && !trainSession.quit; ++currentEpoch)
  {
    double avgLoss = 0;
    uint64_t trainedFramesPerEpoch = 0;
    info.currentEpoch = currentEpoch;

    for (std::vector<std::string>::size_type currentFileIndex = 0;
      currentFileIndex < trainingFiles.size() && !trainSession.quit;
      ++currentFileIndex)
    {
      std::string currentFileName = trainingFiles[currentFileIndex];
      ssbml::video_file videoFile(currentFileName + ".mp4");
      ssbml::gamepad_file gamepadFile(currentFileName + ".gamepad");
      uint64_t currentFileFrameCount = videoFile.get_total_frames();
      uint64_t batches = currentFileFrameCount / trainSession.batchSize;
      uint64_t excessFrames = currentFileFrameCount - batches
        * trainSession.batchSize;
      if (excessFrames == 0)
      {
        --batches;
        excessFrames += 10;
      }
      info.currentFileName = currentFileName;
      info.currentFileFrameCount = currentFileFrameCount;
      info.currentFileIndex = currentFileIndex;

      for (uint64_t currentBatch = 0; currentBatch < batches
        && !trainSession.quit; ++currentBatch)
      {
        info.fileProgress = (double)currentBatch / batches;
        info.epochProgress = (double)trainedFramesPerEpoch
          / totalFramesPerEpoch;
        info.totalProgress = (double)trainedFrames / totalFrames;
        info.currentFrame = currentBatch * trainSession.batchSize;
        info.timeTaken = timer.total_time();
        info.eta = (double)info.timeTaken / trainedFrames
          * (totalFrames - trainedFrames);
        info.eta = ((info.eta + 10000000 / 2) / 10000000) * 10000000;
        trainSession.set_info(info);

        childProgram.write_to((uint8_t)to_child_flag::train_batch_request);

        for (uint64_t currentFrame = 0; currentFrame < trainSession.batchSize;
          ++currentFrame)
        {
          videoFile >> rgbBuf;
          gamepadFile >> gamepadBuf;
          gamepadFile >> labelBuf;
          gamepadFile.rewind();
          childProgram.write_to(buf, rgbBufSize + gamepadBufSize * 2);
        }

        while (!trainSession.quit && (received = childProgram.try_read_from(buf,
          bufSize)) < 0)
          ;
        if (received > 0)
        {
          if (buf[0] == (uint8_t)from_child_flag::train_batch_request_ack)
          {
            avgLoss += ((float*)(buf + 1))[0];
          }
          else
          {
            std::cerr << "Received unknown flag 0x" << std::setfill('0')
              << std::setw(2) << std::hex << (uint32_t)buf[0] << std::endl;
          }
        }
        trainedFrames += trainSession.batchSize;
        trainedFramesPerEpoch += trainSession.batchSize;
      }
      trainedFrames += excessFrames;
      trainedFramesPerEpoch += excessFrames;
    }

    avgLoss /= totalFramesPerEpoch;
    std::cout << avgLoss << std::endl;
  }

  info.fileProgress = 1.0;
  info.epochProgress = 1.0;
  info.totalProgress = 1.0;
  info.currentEpoch = trainSession.totalEpochs;
  info.currentFileName = "";
  info.timeTaken = timer.total_time();
  info.eta = 0;

  if (!trainSession.quit)
  {
    childProgram.write_to(
      (uint8_t)ssbml::train_session::to_child_flag::save_model_request);
  }

  while (!trainSession.quit && (received = childProgram.try_read_from(buf,
    bufSize)) < 0)
  {
    trainSession.set_info(info);
    timer.get_delta_time(66666);
  }
  if (received > 0)
  {
    uint8_t flag = buf[0];
    switch (flag)
    {
      case (uint8_t)ssbml::train_session::from_child_flag::save_model_request_ack:
        break;
      default:
        std::cerr << "Received unknown flag 0x" << std::setfill('0')
          << std::setw(2) << std::hex << (uint32_t)flag << std::endl;
        delete[] buf;
        return;
    }
  }

  if(!trainSession.quit)
  {
    info.trainingCompleted = true;
    trainSession.set_info(info);
    if (trainSession.suspendOnCompletion)
    {
      std::system("systemctl suspend");
    }
  }
  delete[] buf;
}
