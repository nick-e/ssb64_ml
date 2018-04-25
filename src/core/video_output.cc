#include "video_output.h"

ssbml::video_output::~video_output()
{
  int ret = 0;
  if ((ret = avcodec_send_frame(cctx, NULL)) < 0)
  {
    std::cerr << "Error occured when flushing the codec" << std::endl;
  }
  while (true)
  {
    ret = avcodec_receive_packet(cctx, pkt);
    if (ret < 0)
    {
      if (ret != AVERROR_EOF)
      {
        std::cerr << "Error occured while writing packet" << std::endl;
      }
      break;
    }
    av_packet_rescale_ts(pkt, cctx->time_base, stream->time_base);
    pkt->stream_index = stream->index;
    ret = av_interleaved_write_frame(fctx, pkt);
    if (av_interleaved_write_frame(fctx, pkt) < 0)
    {
      std::cerr << "Failed to write frame" << std::endl;
      break;
    }
  }

  av_frame_free(&frame);
  av_packet_free(&pkt);
  av_write_trailer(fctx);
  avcodec_free_context(&cctx);

  if (~(fmt->flags & AVFMT_NOFILE))
  {
    avio_close(fctx->pb);
  }
  avformat_free_context(fctx);
}

ssbml::video_output::video_output(std::string fileName, uint64_t width,
  uint64_t height, double fps) :
  nextPts(0)
{
  int ret;
  // initialize container
  if (!(fmt = av_guess_format(NULL, fileName.c_str(), NULL)))
  {
    throw std::runtime_error("Could not deduce output format from file " \
      "extesion");
  }
  if (!(fctx = avformat_alloc_context()))
  {
    throw std::runtime_error("Failed to allocate AVFormatContext");
  }
  fctx->oformat = fmt;
  snprintf(fctx->filename, sizeof(fctx->filename), "%s", fileName.c_str());

  // add video stream to container
  AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  if (!codec)
  {
    avformat_free_context(fctx);
    throw std::runtime_error("Could not find codec");
  }
  if (!(stream = avformat_new_stream(fctx, NULL)))
  {
    avformat_free_context(fctx);
    throw std::runtime_error("Could not allocate AVStream");
  }
  if (!(cctx = avcodec_alloc_context3(codec)))
  {
    avformat_free_context(fctx);
    throw std::runtime_error("Coudl not allocate AVCodecContext");
  }
  cctx->bit_rate = 1000000;
  cctx->width = width;
  cctx->height = height;
  cctx->time_base.num = 1;
  cctx->time_base.den = (int)fps;
  //stream->time_base.num = 1;
  //stream->time_base.den = (int)fps;
  cctx->gop_size = 15;
  cctx->pix_fmt = AV_PIX_FMT_YUV420P;

  if (fctx->oformat->flags & AVFMT_GLOBALHEADER)
  {
    cctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }

  // open video stream
  if ((ret = avcodec_open2(cctx, NULL, NULL)) < 0)
  {
    avcodec_free_context(&cctx);
    avformat_free_context(fctx);
    throw std::runtime_error("Could not open codec");
  }

  // alloc frame
  if ((frame = av_frame_alloc()) == NULL)
  {
    avcodec_free_context(&cctx);
    avformat_free_context(fctx);
    throw std::runtime_error("Could not allocate AVFrame");
  }
  frame->format = cctx->pix_fmt;
  frame->width = width;
  frame->height = height;
  av_frame_get_buffer(frame, 0);

  if ((pkt = av_packet_alloc()) == NULL)
  {
    av_frame_free(&frame);
    avcodec_free_context(&cctx);
    avformat_free_context(fctx);
    throw std::runtime_error("Could not allocate AVPacket");
  }

  if (avcodec_parameters_from_context(stream->codecpar, cctx) < 0)
  {
    av_packet_free(&pkt);
    av_frame_free(&frame);
    avcodec_free_context(&cctx);
    avformat_free_context(fctx);
    throw std::runtime_error("Could not create AVCodecParameters from " \
      "AVCodecContext");
  }
  av_dump_format(fctx, 0, fileName.c_str(), 1);

  // open the output file if needed
  if (!(fmt->flags & AVFMT_NOFILE) && avio_open(&fctx->pb, fileName.c_str(),
    AVIO_FLAG_WRITE) < 0)
  {
    av_packet_free(&pkt);
    av_frame_free(&frame);
    avcodec_free_context(&cctx);
    avformat_free_context(fctx);
    throw std::runtime_error("Could not open file");
  }

  // write stream header
  if (avformat_write_header(fctx, NULL) < 0)
  {
    av_packet_free(&pkt);
    av_frame_free(&frame);
    avcodec_free_context(&cctx);
    avformat_free_context(fctx);
    throw std::runtime_error("Failed to write video header");
  }
}

void ssbml::video_output::write_frame()
{
  int ret;
    frame->pts = nextPts++;
  if ((ret = avcodec_send_frame(cctx, frame)) < 0)
  {
    char errBuf[1024];
    av_strerror(ret, errBuf, sizeof(errBuf));
    throw std::runtime_error("avcodec_send_frame: " + std::string(errBuf));
  }
  timer t;
  while (true)
  {
    ret = avcodec_receive_packet(cctx, pkt);
    if (ret == AVERROR(EAGAIN))
    {
      break;
    }
    if (ret < 0)
    {
      char errBuf[1024];
      av_strerror(ret, errBuf, sizeof(errBuf));
      throw std::runtime_error("avcodec_receive_packet: "
        + std::string(errBuf));
    }
    av_packet_rescale_ts(pkt, cctx->time_base, stream->time_base);
    pkt->stream_index = stream->index;

    timer t;
    if (av_interleaved_write_frame(fctx, pkt) < 0)
    {
      char errBuf[1024];
      av_strerror(ret, errBuf, sizeof(errBuf));
      throw std::runtime_error("av_interleaved_write_frame: "
        + std::string(errBuf));
    }
    std::cout << t.get_delta_time() << std::endl;

    av_packet_unref(pkt);
  }
}
