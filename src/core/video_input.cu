#include "video_input.h"

#define THREAD_COUNT 16
#define DIVCEIL(X, Y) ((X) + (Y) - 1) / (Y)

Window* ssbml::video_input::get_all_visible_windows(Display **display,
  unsigned long *numWindows)
{
  *display = XOpenDisplay(NULL);
  Atom property = XInternAtom(*display, "_NET_CLIENT_LIST", False), type;
  int form;
  unsigned long remaining;
  unsigned char *list;

  errno = 0;
  if (XGetWindowProperty(*display, XDefaultRootWindow(*display), property, 0,
    1024, False, XA_WINDOW, &type, &form, numWindows, &remaining, &list)
    != Success)
  {
    perror("ssbml::video_input::get_all_visible_windows(): " \
      "XGetWindowProperty()");
    return NULL;
  }

  return (Window*)list;
}

uint32_t ssbml::video_input::get_window_pid(Display *display, Window window)
{
  Atom property = XInternAtom(display, "_NET_WM_PID", False), type;
  uint32_t *pidContainer, pid;
  int form;
  unsigned long remaining, bytes;
  if (XGetWindowProperty(display, window, property, 0, 1, False, XA_CARDINAL,
    &type, &form, &bytes, &remaining, (unsigned char**)(&pidContainer))
    != Success)
  {
    perror("ssbml::video_input::get_window_pid(): XGetWindowProperty()");
    return 0;
  }
  pid = *pidContainer;
  XFree(pidContainer);
  return pid;
}

std::string ssbml::video_input::get_process_name(uint32_t pid)
{
  std::string processName;
  std::ifstream cmdline("/proc/" + std::to_string(pid) + "/cmdline");
  std::getline(cmdline, processName);
  cmdline.close();
  return processName;
}

std::string ssbml::video_input::get_window_title(Display *display,
  Window window)
{
  char *windowTitle;
  if (XFetchName(display, window, &windowTitle))
  {
    std::string tmp(windowTitle);
    XFree(windowTitle);
    return std::string(tmp);
  }
  return "";
}

ssbml::video_input::video_input(Display *display, Window window,
  uint64_t frameWidth, uint64_t frameHeight) :
  display(display),
  window(window),
  frameWidth(frameWidth),
  frameHeight(frameHeight),
  frameSize(frameWidth * frameHeight),
  chromaSize(frameWidth * frameHeight / 4),
  xcbCon(xcb_connect(DisplayString(display), NULL))
{
  if (xcb_connection_has_error(xcbCon) > 0)
  {
    throw std::runtime_error("Could not connect to X server");
  }

  XGetWindowAttributes(display, window, &windowAttributes);

  cudaMalloc(&rgbaBuf1, windowAttributes.width * windowAttributes.height * 4);
  cudaMalloc(&rgbaBuf2, windowAttributes.width * windowAttributes.height * 4);
  cudaMalloc(&rgbBuf, windowAttributes.width * windowAttributes.height * 3);
  cudaMalloc(&lumaBuf, frameSize);
  cudaMalloc(&redChromaBuf, frameSize);
  cudaMalloc(&blueChromaBuf, frameSize);
}

ssbml::video_input::~video_input()
{
  xcb_disconnect(xcbCon);
  cudaFree(rgbaBuf1);
  cudaFree(rgbaBuf2);
  cudaFree(rgbBuf);
  cudaFree(lumaBuf);
  cudaFree(redChromaBuf);
  cudaFree(blueChromaBuf);
}

__global__ void reduce_width(uint8_t *rgbaBuf_in, uint8_t *rgbaBuf_out,
  int width, int height)
{
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;
  int half = width / 2;

  if (col < half && row < height)
  {
    uint32_t p1 = 0, p2 = 0;
    int col2 = col * 2;
    p1 = ((uint32_t*)rgbaBuf_in)[row * width + col2];
    p2 = ((uint32_t*)rgbaBuf_in)[row * width + (col2 + 1)];
    ((uint32_t*)rgbaBuf_out)[row * half + col] = __vavgu4(p1, p2);
  }
}

__global__ void reduce_height(uint8_t *rgbaBuf_in, uint8_t *rgbaBuf_out,
  uint64_t width, uint64_t height)
{
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;
  int half = height / 2;

  if (col < width && row < half)
  {
    uint32_t p1 = 0, p2 = 0;
    int row2 = row * 2;
    p1 = ((uint32_t*)rgbaBuf_in)[row2 * width + col];
    p2 = ((uint32_t*)rgbaBuf_in)[(row2 + 1) * width + col];
    ((uint32_t*)rgbaBuf_out)[row * width + col] = __vavgu4(p1, p2);
  }
}

__device__ float lerp(float s, float e, float t)
{
  return s + (e - s) * t;
}

__device__ float blerp(float c00, float c10, float c01, float c11, float tx, float ty)
{
  return lerp(lerp(c00, c10, tx), lerp(c01, c11, tx), ty);
}

__global__ void rgba2yuv(const uint8_t *rgbaBuf, uint8_t *lumaBuf,
  uint8_t *redChromaBuf, uint8_t *blueChromaBuf, uint64_t width,
  uint64_t height)
{
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;
  if (col < width && row < height)
  {
    int index = row * width + col;
    int index2 = index * 4;
    lumaBuf[index] = 0.299 * rgbaBuf[index2] + 0.587 * rgbaBuf[index2 + 1]
      + 0.114 * rgbaBuf[index2 + 2];

    int col2 = col * 2, row2 = row * 2;
    if (col2 < width && row2 < height)
    {
      int i00 = row2 * width + col2;
      int i01 = (row2 + 1) * width + col2;
      int i10 = row2 * width + (col2 + 1);
      int i11 = (row2 + 1) * width + (col2 + 1);

      uint32_t pixel = __vavgu4(
        ((uint32_t*)rgbaBuf)[i00],
        __vavgu4(
          ((uint32_t*)rgbaBuf)[i01],
          __vavgu4(
            ((uint32_t*)rgbaBuf)[i10],
            ((uint32_t*)rgbaBuf)[i11]
          )
        )
      );

      uint8_t blue = (uint8_t)(pixel >> 16);
      uint8_t green = (uint8_t)(pixel >> 8);
      uint8_t red = (uint8_t)pixel;

      index = row * (width / 2) + col;
      blueChromaBuf[index] = (-0.168736 * red + -0.331264 * green + 0.5 * blue)
        + 128;
      redChromaBuf[index] = (0.5 * red + -0.418688 * green + -0.081312 * blue)
        + 128;
    }
  }
}

__global__ void rgba2rgb(const uint8_t *rgbaBuf, uint8_t *rgbBuf,
  uint64_t width, uint64_t height)
{
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;
  if (col < width && row < height)
  {
    int index = row * width + col;
    int index2 = index * 3;
    uint32_t pixel = ((uint32_t*)rgbaBuf)[index];
    uint8_t red = (uint8_t)(pixel >> 16);
    uint8_t green = (uint8_t)(pixel >> 8);
    uint8_t blue = (uint8_t)pixel;

    rgbBuf[index2] = red;
    rgbBuf[index2 + 1] = green;
    rgbBuf[index2 + 2] = blue;
  }
}

// rgbaBuf initially has rgba but will end with just rgb
__global__ void bilinear(uint8_t *rgbaBuf_in, uint8_t *rgbaBuf_out,
  uint64_t newWidth, uint64_t newHeight, uint64_t oldWidth, uint64_t oldHeight)
{
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;
  int in = col < newWidth && row < newHeight;
  uint8_t red = 0, green = 0, blue = 0;
  if (in)
  {
    float gy = row / (float)newHeight * (oldHeight - 1);
    int gyi = (int)gy;
    float gx = col / (float)newWidth * (oldWidth - 1);
    int gxi = (int)gx;
    float dx = gx - gxi, dy = gy - gyi;

    uint8_t *p00 = rgbaBuf_in + (gyi * oldWidth + gxi) * 4;
    uint8_t *p10 = rgbaBuf_in + (gyi * oldWidth + gxi + 1) * 4;
    uint8_t *p01 = rgbaBuf_in + ((gyi + 1) * oldWidth + gxi) * 4;
    uint8_t *p11 = rgbaBuf_in + ((gyi + 1) * oldWidth + gxi + 1) * 4;

    red = (uint8_t)blerp(p00[2], p10[2], p01[2], p11[2], dx, dy);
    green = (uint8_t)blerp(p00[1], p10[1], p01[1], p11[1], dx, dy);
    blue = (uint8_t)blerp(p00[0], p10[0], p01[0], p11[0], dx, dy);

    int index = (row * newWidth + col) * 4;
    rgbaBuf_out[index] = red;
    rgbaBuf_out[index + 1] = green;
    rgbaBuf_out[index + 2] = blue;
    rgbaBuf_out[index + 3] = 255;
  }
}

void ssbml::video_input::get_image()
{
  xcb_get_image_cookie_t cookie;
  xcb_get_image_reply_t *reply;
  xcb_generic_error_t *err = NULL;
  uint64_t width = windowAttributes.width, height = windowAttributes.height;
  uint8_t *imageData;

  cookie = xcb_get_image(xcbCon, XCB_IMAGE_FORMAT_Z_PIXMAP, window, 0, 0, width, height, ~0);
  reply = xcb_get_image_reply(xcbCon, cookie, &err);
  if (err)
  {
    std::cerr << "Cannot grab window image data. response type="
      << err->response_type << " error code=" << err->error_code << " sequence="
      << err->sequence << " resource id=" << err->resource_id << " minor code="
      << err->minor_code << " major code=" << err->major_code << std::endl;
  }
  if (!reply)
  {
    std::cerr << "Reply was empty" << std::endl;
  }
  imageData = xcb_get_image_data(reply);
  cudaMemcpy(rgbaBuf1, imageData, height * width * 4, cudaMemcpyHostToDevice);
  free(reply);

  /*XImage *image = XGetImage(display, window, 0, 0, width, height, AllPlanes,
    ZPixmap);
  cudaMemcpy(rgbaBuf1, image->data, height * width * 4, cudaMemcpyHostToDevice);
  XFree(image);*/

  uint8_t *rgbaBuf_in = rgbaBuf1, *rgbaBuf_out = rgbaBuf2;
  dim3 threads(THREAD_COUNT, THREAD_COUNT);
  while (width / 2 >= frameWidth)
  {
    reduce_width<<<dim3(DIVCEIL(width / 2, THREAD_COUNT),
      DIVCEIL(height, THREAD_COUNT)), threads>>>(rgbaBuf_in, rgbaBuf_out, width,
      height);
    cudaDeviceSynchronize();
    width /= 2;
    std::swap(rgbaBuf_in, rgbaBuf_out);
  }
  while (height / 2 >= frameHeight)
  {
    reduce_height<<<dim3(DIVCEIL(width, THREAD_COUNT),
      DIVCEIL(height / 2, THREAD_COUNT)), threads>>>(rgbaBuf_in, rgbaBuf_out,
      width, height);
    cudaDeviceSynchronize();
    height /= 2;
    std::swap(rgbaBuf_in, rgbaBuf_out);
  }

  if (height != frameHeight && width != frameWidth)
  {
    dim3 blocks(DIVCEIL(frameWidth, THREAD_COUNT),
      DIVCEIL(frameHeight, THREAD_COUNT));
    bilinear<<<blocks, threads>>>(rgbaBuf_in, rgbaBuf_out, frameWidth,
      frameHeight, width, height);
    std::swap(rgbaBuf_in, rgbaBuf_out);
    cudaDeviceSynchronize();
  }
  rgbaBuf = rgbaBuf_in;
}

ssbml::video_input& ssbml::video_input::operator>>(video_output &videoOutput)
{
  get_frame(*videoOutput.frame);
  videoOutput.write_frame();
  return *this;
}

ssbml::video_input& ssbml::video_input::operator>>(uint8_t *rgbBuf)
{
  get_frame(rgbBuf);
  return *this;
}

void ssbml::video_input::get_frame(uint8_t *rgbBuf)
{
  get_image();
  dim3 threads(THREAD_COUNT, THREAD_COUNT);
  dim3 blocks(DIVCEIL(frameWidth, THREAD_COUNT),
    DIVCEIL(frameHeight, THREAD_COUNT));
  rgba2rgb<<<blocks, threads>>>(rgbaBuf, this->rgbBuf, frameWidth, frameHeight);
  cudaDeviceSynchronize();

  cudaMemcpy(rgbBuf, this->rgbBuf, frameSize * 3, cudaMemcpyDeviceToHost);
}

void ssbml::video_input::get_frame(AVFrame &frame)
{
  get_image();
  dim3 threads(THREAD_COUNT, THREAD_COUNT);
  dim3 blocks((frameWidth + THREAD_COUNT - 1 ) / THREAD_COUNT,
    (frameHeight + THREAD_COUNT - 1) / THREAD_COUNT);
  rgba2yuv<<<blocks, threads>>>(rgbaBuf, lumaBuf, redChromaBuf, blueChromaBuf,
    frameWidth, frameHeight);
  cudaDeviceSynchronize();

  av_frame_make_writable(&frame);
  cudaMemcpy(frame.data[0], lumaBuf, frameSize, cudaMemcpyDeviceToHost);
  cudaMemcpy(frame.data[1], blueChromaBuf, chromaSize, cudaMemcpyDeviceToHost);
  cudaMemcpy(frame.data[2], redChromaBuf, chromaSize, cudaMemcpyDeviceToHost);
}
