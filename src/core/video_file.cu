#include "video_file.h"

#define THREAD_COUNT 16
#define DIVCEIL(X, Y) ((X) + (Y) - 1) / (Y)

ssbml::video_file::~video_file()
{
  av_frame_free(&frame);
  av_packet_free(&pkt);
  avcodec_close(cctx);
  avformat_close_input(&fctx);

  cudaFree(rgbBuf);
  cudaFree(lumaBuf);
  cudaFree(redChromaBuf);
  cudaFree(blueChromaBuf);
}

ssbml::video_file::video_file(std::string fileName) : nextFrame(0), fctx(NULL),
  cctx(NULL), codec(NULL), frame(NULL)
{
  if (avformat_open_input(&fctx, fileName.c_str(), NULL, NULL) != 0)
  {
    throw std::runtime_error("Failed to open file");
  }
  if (avformat_find_stream_info(fctx, NULL) < 0)
  {
    avformat_close_input(&fctx);
    throw std::runtime_error("Couldn't find stream information");
  }

  av_dump_format(fctx, 0, fileName.c_str(), 0);

  AVStream *stream = nullptr;
  for (uint64_t i = 0; i < fctx->nb_streams; ++i)
  {
    if (fctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
    {
      stream = fctx->streams[i];
      break;
    }
  }
  if (stream == nullptr)
  {
    avformat_close_input(&fctx);
    avcodec_close(cctx);
    throw std::runtime_error("File does not contain a video stream");
  }
  if ((codec = avcodec_find_decoder(stream->codecpar->codec_id)) == NULL)
  {
    avformat_close_input(&fctx);
    throw std::runtime_error("File uses unsupported codec");
  }
  if ((cctx = avcodec_alloc_context3(codec)) == NULL)
  {
    avformat_close_input(&fctx);
    throw std::runtime_error("Failed to allocate AVCodecContext");
  }
  if (avcodec_parameters_to_context(cctx, stream->codecpar) < 0)
  {
    avformat_close_input(&fctx);
    throw std::runtime_error("Failed to convert codec parameters to a " \
      "codec context");
  }
  uint64_t fps = stream->avg_frame_rate.num / stream->avg_frame_rate.den;
  totalFrames = (fctx->duration * fps) / 1000000;

  if (avcodec_open2(cctx, codec, NULL) < 0)
  {
    avcodec_close(cctx);
    avformat_close_input(&fctx);
    throw std::runtime_error("Failed to open codec");
  }
  if ((frame = av_frame_alloc()) == NULL)
  {
    avcodec_close(cctx);
    avformat_close_input(&fctx);
    throw std::runtime_error("Failed to allocated AVFrame");
  }

  if ((pkt = av_packet_alloc()) == NULL)
  {
    av_frame_free(&frame);
    avcodec_close(cctx);
    avformat_close_input(&fctx);
    throw std::runtime_error("Could not allocate AVPacket");
  }

  frameSize = cctx->width * cctx->height;
  cudaMalloc(&rgbBuf, frameSize * 3);
  cudaMalloc(&lumaBuf, frameSize);
  cudaMalloc(&redChromaBuf, frameSize);
  cudaMalloc(&blueChromaBuf, frameSize);
}

uint64_t ssbml::video_file::get_total_frames()
{
  return totalFrames;
}

uint64_t ssbml::video_file::get_frame_width()
{
  return cctx->width;
}

uint64_t ssbml::video_file::get_frame_height()
{
  return cctx->height;
}

ssbml::video_file& ssbml::video_file::operator>>(uint8_t *rgbBuf)
{
  get_frame(rgbBuf);
  return *this;
}

ssbml::video_file& ssbml::video_file::operator>>(video_output &videoOutput)
{
  get_frame(*videoOutput.frame);
  return *this;
}

__global__ void yuv2rgb(uint8_t *rgbBuf, const uint8_t *lumaBuf,
  const uint8_t *redChromaBuf, const uint8_t *blueChromaBuf, uint64_t width,
  uint64_t height)
{
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;

  if (col < width && row < height)
  {
    int index = row * width + col;
    int index2 = index * 3;
    int chromaIndex = (row / 2) * (width / 2) + (col / 2);

    uint8_t luma = lumaBuf[index];
    uint8_t redChroma = redChromaBuf[chromaIndex];
    uint8_t blueChroma = blueChromaBuf[chromaIndex];

    uint16_t tmp = luma + 1.4075 * (redChroma - 128);
    uint8_t red = tmp > 255 ? 255 : tmp;
    tmp = luma - 0.3455 * (blueChroma - 128) - (0.7169 * (redChroma - 128));
    uint8_t green = tmp > 255 ? 255 : tmp;
    tmp = luma + 1.7790 * (blueChroma - 128);
    uint8_t blue = tmp > 255 ? 255 : tmp;

    rgbBuf[index2] = red;
    rgbBuf[index2 + 1] = green;
    rgbBuf[index2 + 2] = blue;
  }
}

void ssbml::video_file::get_image()
{
  int ret;
  if (nextFrame < totalFrames)
  {
    ret = av_read_frame(fctx, pkt);
    if (ret < 0)
    {
      char errBuf[1024];
      av_strerror(ret, errBuf, sizeof(errBuf));
      throw std::runtime_error("av_read_frame: " + std::string(errBuf));
    }


    ret = avcodec_send_packet(cctx, pkt);
    if (ret < 0)
    {
      char errBuf[1024];
      av_strerror(ret, errBuf, sizeof(errBuf));
      throw std::runtime_error("avcodec_send_packet: " + std::string(errBuf));
    }

    while (ret >= 0)
    {
      ret = avcodec_receive_frame(cctx, frame);
      if (ret == AVERROR(EAGAIN))
      {
        break;
      }
      if (ret < 0)
      {
        char errBuf[1024];
        av_strerror(ret, errBuf, sizeof(errBuf));
        throw std::runtime_error("avcodec_receive_frame: " + std::string(errBuf));
      }
    }

    av_packet_unref(pkt);
    ++nextFrame;
  }
  else
  {
    throw std::runtime_error("Cannot get more frames, end of file");
  }
}

void ssbml::video_file::get_frame(uint8_t *rgbBuf)
{
  get_image();

  cudaMemcpy(lumaBuf, frame->data[0], frameSize, cudaMemcpyHostToDevice);
  cudaMemcpy(blueChromaBuf, frame->data[1], frameSize / 4, cudaMemcpyHostToDevice);
  cudaMemcpy(redChromaBuf, frame->data[2], frameSize / 4, cudaMemcpyHostToDevice);
  dim3 threads(THREAD_COUNT, THREAD_COUNT);
  dim3 blocks(DIVCEIL(cctx->width, THREAD_COUNT),
    DIVCEIL(cctx->height, THREAD_COUNT));
  yuv2rgb<<<blocks, threads>>>(this->rgbBuf, lumaBuf, redChromaBuf, blueChromaBuf,
    cctx->width, cctx->height);
  cudaDeviceSynchronize();

  cudaMemcpy(rgbBuf, this->rgbBuf, frameSize * 3, cudaMemcpyDeviceToHost);
}

void ssbml::video_file::get_frame(AVFrame &frame)
{
  get_image();

  av_frame_make_writable(&frame);
  memcpy(frame.data[0], this->frame->data[0], frameSize);
  memcpy(frame.data[1], this->frame->data[1], frameSize / 4);
  memcpy(frame.data[2], this->frame->data[2], frameSize / 4);
}
