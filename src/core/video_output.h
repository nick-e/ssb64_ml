#ifndef VIDEO_OUTPUT_H
#define VIDEO_OUTPUT_H

#include <stdint.h>
#include <string>
#include <string.h>
#include <iostream>
#include <stdexcept>

extern "C"
{
  #include <libavformat/avformat.h>
  #include <libavcodec/avcodec.h>
  #include <libavutil/error.h>
}

#include "timer.h"

namespace ssbml
{
  class video_output
  {
    friend class video_input;
    friend class video_file;
  public:
    video_output(std::string fileName, uint64_t width, uint64_t height,
      double fps);
    ~video_output();
    void write_frame();

  protected:
    AVFrame *frame;
    uint64_t nextPts;
    AVStream *stream;
    AVCodecContext *cctx;
    AVFormatContext *fctx;
    AVOutputFormat *fmt;
    AVPacket *pkt;
    FILE *fp;
  };
}

#endif
