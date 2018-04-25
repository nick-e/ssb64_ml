#ifndef VIDEO_FILE_H
#define VIDEO_FILE_H

#include <string>
#include <cstdint>
#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <cstring>

extern "C"
{
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
  #include <libavutil/error.h>
}

#include <cuda.h>

#include "video_output.h"

namespace ssbml
{
  class video_file
  {
  public:
    video_file(std::string fileName);
    ~video_file();

    video_file& operator>>(uint8_t *rgbBuf);
    video_file& operator>>(video_output &videoOutput);

    void get_frame(uint8_t *rgbBuf);
    void get_frame(AVFrame &frame);
    uint64_t get_frame_width();
    uint64_t get_frame_height();
    uint64_t get_total_frames();

  private:
    uint64_t totalFrames, nextFrame;
    AVFormatContext *fctx;
    AVCodecContext *cctx;
    AVCodec *codec;
    AVFrame *frame;
    AVPacket *pkt;
    uint64_t frameSize;
    uint8_t *lumaBuf, *redChromaBuf, *blueChromaBuf, *rgbBuf;

    void get_image();
  };
}

#endif
