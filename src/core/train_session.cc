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
    + " 128 72").c_str(), "r");
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

static uint64_t get_files(std::string directory,
  std::vector<std::string> &fileNames)
{
  uint64_t totalFrameCount = 0;

  DIR *dir = opendir(directory.c_str());
  struct dirent *entry;
  if (dir == NULL)
  {
    throw std::runtime_error(std::string("opendir: ") + strerror(errno));
  }
  while ((entry = readdir(dir)) != NULL)
  {
    struct stat s;
    std::string videoFileName = directory + "/" + entry->d_name;

    if (!has_suffix(videoFileName, ".mp4"))
    {
      continue;
    }
    if (stat(videoFileName.c_str(), &s) < 0)
    {
      throw std::runtime_error(std::string("stat: ") + strerror(errno));
    }
    if ((s.st_mode & S_IFMT) != S_IFREG)
    {
      continue;
    }

    ssbml::video_file videoFile(videoFileName);
    fileNames.push_back(videoFileName.substr(0, videoFileName.find_last_of(".")));
    totalFrameCount += videoFile.get_total_frames();;
  }
  closedir(dir);

  return totalFrameCount;
}

ssbml::train_session::train_session(std::string metaFile,
	std::string trainingDataDir, bool suspendOnCompletion,
	uint64_t downsampleRate, uint64_t frameWidth, uint64_t frameHeight,
	uint64_t lookback, uint64_t totalEpochs, Glib::Dispatcher &dispatcher) :
  dispatcher(dispatcher),
  suspendOnCompletion(suspendOnCompletion),
	downsampleRate(downsampleRate),
  frameHeight(frameHeight),
  frameWidth(frameWidth),
	lookback(lookback),
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

static void get_data(uint64_t rate, ssbml::video_file &videoFile,
	ssbml::gamepad_file &gamepadFile, uint8_t *rgbBuf, uint8_t *gamepadBuf)
{
	int16_t buttons;
	int32_t x;
	int32_t y;
	int32_t z;
	int32_t rx;
	int32_t ry;
	int32_t rz;
	float hat0x;
	float hat0y;
	ssbml::gamepad::compressed tmp;

	videoFile >> rgbBuf;
	gamepadFile >> tmp;
	buttons = tmp.buttons;
	x = tmp.analogs.x;
	y = tmp.analogs.y;
	z = tmp.analogs.z;
	rx = tmp.analogs.rx;
	ry = tmp.analogs.ry;
	rz = tmp.analogs.rz;
	hat0x = tmp.analogs.hat0x;
	hat0y = tmp.analogs.hat0y;

	for (uint64_t i = 1; i < rate; ++i)
	{
		gamepadFile >> tmp;
		videoFile.skip_frame();
		buttons |= tmp.buttons;
		x += tmp.analogs.x;
		y += tmp.analogs.y;
		z += tmp.analogs.z;
		rx += tmp.analogs.rx;
		ry += tmp.analogs.ry;
		rz += tmp.analogs.rz;
		hat0x += tmp.analogs.hat0x;
		hat0y += tmp.analogs.hat0y;
	}

	x /= rate;
	y /= rate;
	z /= rate;
	rx /= rate;
	ry /= rate;
	rz /= rate;
	hat0x /= rate;
	hat0y /= rate;

	tmp.buttons = buttons;
	tmp.analogs.x = x;
	tmp.analogs.y = y;
	tmp.analogs.z = z;
	tmp.analogs.rx = rx;
	tmp.analogs.ry = ry;
	tmp.analogs.rz = rz;
	tmp.analogs.hat0x = (hat0x < -0.5) ? -1 : (hat0x > 0.5 ? 1 : 0);
	tmp.analogs.hat0y = (hat0y <= -0.5) ? -1 : (hat0y >= 0.5 ? 1 : 0);

	memcpy(gamepadBuf, &tmp, sizeof(tmp));
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
	uint64_t batchSize = trainSession.lookback * 40 / trainSession.downsampleRate;
  uint64_t gamepadBufSize = sizeof(ssbml::gamepad::compressed);
  uint64_t rgbBufSize = trainSession.frameWidth * trainSession.frameHeight * 3;
  uint64_t bufSize = rgbBufSize + gamepadBufSize * 2;
  uint64_t totalFrameCount = 0;
  uint64_t epochFrameCount = 0;
  uint64_t trainedTotalFrameCount = 0;
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
  epochFrameCount = get_files(trainSession.trainingDataDir, trainingFiles);
  totalFrameCount = epochFrameCount * trainSession.totalEpochs;
  info.totalFiles = trainingFiles.size();

  sprintf(tmp1, "%s", modelName.c_str());
  sprintf(tmp2, "%s", std::to_string(trainSession.frameWidth).c_str());
  sprintf(tmp3, "%s", std::to_string(trainSession.frameHeight).c_str());
  sprintf(tmp4, "%s", std::to_string(batchSize).c_str());
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

	try
	{
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
		uint64_t trainedBatches = 0;
    uint64_t trainedEpochFrames = 0;
    info.currentEpoch = currentEpoch;
    std::random_shuffle(trainingFiles.begin(), trainingFiles.end());

    uint64_t fileIndex = 0;
    for (std::string fileName : trainingFiles)
    {
      ssbml::video_file videoFile(fileName + ".mp4");
      ssbml::gamepad_file gamepadFile(fileName + ".gamepad");
      uint64_t fileFrameCount = videoFile.get_total_frames();
      info.currentFileName = fileName;
      info.currentFileFrameCount = fileFrameCount;
      info.currentFileIndex = fileIndex++;

      if (fileFrameCount < batchSize + 1)
      {
        trainedTotalFrameCount += fileFrameCount;
        trainedEpochFrames += fileFrameCount;
        continue;
      }

      childProgram.write_to((uint8_t)to_child_flag::new_file);
      gamepadFile.get_next();
      for (uint64_t i = 0; i < batchSize - 1 && !trainSession.quit; ++i)
      {
				get_data(trainSession.downsampleRate, videoFile, gamepadFile, rgbBuf,
					gamepadBuf);
        childProgram.write_to(rgbBuf, rgbBufSize);
      }
      trainedTotalFrameCount += (batchSize - 1) * trainSession.downsampleRate;
      trainedEpochFrames += (batchSize - 1) * trainSession.downsampleRate;;
      info.currentFrame = (batchSize - 1) * trainSession.downsampleRate;;
      info.fileProgress = (double)info.currentFrame / fileFrameCount;
      info.epochProgress = (double)trainedEpochFrames / epochFrameCount;
      info.totalProgress = (double)trainedTotalFrameCount / totalFrameCount;
      info.timeTaken = timer.total_time();
      info.eta = 0;
      trainSession.set_info(info);

      for (uint64_t frameIndex = batchSize * trainSession.downsampleRate;
        frameIndex < fileFrameCount - trainSession.downsampleRate
					&& !trainSession.quit;
				frameIndex += trainSession.downsampleRate)
      {
        childProgram.write_to((uint8_t)to_child_flag::train);
				get_data(trainSession.downsampleRate, videoFile, gamepadFile, rgbBuf,
					gamepadBuf);
        childProgram.write_to(buf, rgbBufSize + gamepadBufSize);
        trainedTotalFrameCount += trainSession.downsampleRate;
        trainedEpochFrames += trainSession.downsampleRate;
				++trainedBatches;
        info.currentFrame = frameIndex;
        info.fileProgress = (double)info.currentFrame / fileFrameCount;
        info.epochProgress = (double)trainedEpochFrames / epochFrameCount;
        info.totalProgress = (double)trainedTotalFrameCount / totalFrameCount;
        info.timeTaken = timer.total_time();
        info.eta = 0;
        trainSession.set_info(info);

        while (!trainSession.quit && (received = childProgram.try_read_from(buf,
          bufSize)) < 0)
          ;
        if (received > 0)
        {
          avgLoss += ((float*)buf)[0];
					std::cout << "\r" << (avgLoss / trainedBatches) << std::flush;
        }
      }

			trainedTotalFrameCount += fileFrameCount % trainSession.downsampleRate;
			trainedEpochFrames += fileFrameCount % trainSession.downsampleRate;
    }

    avgLoss /= trainedBatches;
    std::cout << "train loss = " << avgLoss << std::endl;
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
    childProgram.write_to((uint8_t)ssbml::train_session::to_child_flag::save);
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
      case (uint8_t)ssbml::train_session::from_child_flag::save_ack:
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
			std::cout << "called suspend" << std::endl;
      std::system("systemctl suspend");
    }
  }
	}
	catch (const std::runtime_error &e)
	{
		std::cout << e.what() << std::endl;
	}
  delete[] buf;
}
