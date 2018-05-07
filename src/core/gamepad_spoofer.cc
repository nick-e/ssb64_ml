#include "gamepad_spoofer.h"

ssbml::gamepad_spoofer& ssbml::gamepad_spoofer::operator<<(const compressed &c)
{
  update(c);
  return *this;
}

ssbml::gamepad_spoofer& ssbml::gamepad_spoofer::operator<<(const gamepad &g)
{
  update(g);
  return *this;
}

ssbml::gamepad_spoofer& ssbml::gamepad_spoofer::operator<<(threadsafe_gamepad &g)
{
  update(g);
  return *this;
}

void ssbml::gamepad_spoofer::update(const compressed &c)
{
  gamepad g(c);
  update(g);
}

void ssbml::gamepad_spoofer::update(threadsafe_gamepad &g)
{
  std::lock_guard<std::mutex> lock(g.m);
  update((gamepad)g);
}

void ssbml::gamepad_spoofer::update(const gamepad &g)
{
  struct input_event ie = syn;
  if (this->buttons.a != g.buttons.a)
  {
    this->buttons.a = g.buttons.a;
    ie.type = EV_KEY;
    ie.code = BTN_A;
    ie.value = this->buttons.a;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->buttons.b != g.buttons.b)
  {
    this->buttons.b = g.buttons.b;
    ie.type = EV_KEY;
    ie.code = BTN_B;
    ie.value = this->buttons.b;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->buttons.x != g.buttons.x)
  {
    this->buttons.x = g.buttons.x;
    ie.type = EV_KEY;
    ie.code = BTN_X;
    ie.value = this->buttons.x;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->buttons.y != g.buttons.y)
  {
    this->buttons.y = g.buttons.y;
    ie.type = EV_KEY;
    ie.code = BTN_Y;
    ie.value = this->buttons.y;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->buttons.tr != g.buttons.tr)
  {
    this->buttons.tr = g.buttons.tr;
    ie.type = EV_KEY;
    ie.code = BTN_TR;
    ie.value = this->buttons.tr;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->buttons.tl != g.buttons.tl)
  {
    this->buttons.tl = g.buttons.tl;
    ie.type = EV_KEY;
    ie.code = BTN_TL;
    ie.value = this->buttons.tl;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->buttons.thumbr != g.buttons.thumbr)
  {
    this->buttons.thumbr = g.buttons.thumbr;
    ie.type = EV_KEY;
    ie.code = BTN_THUMBR;
    ie.value = this->buttons.thumbr;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->buttons.thumbl != g.buttons.thumbl)
  {
    this->buttons.thumbl = g.buttons.thumbl;
    ie.type = EV_KEY;
    ie.code = BTN_THUMBL;
    ie.value = this->buttons.thumbl;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->buttons.select != g.buttons.select)
  {
    this->buttons.select = g.buttons.select;
    ie.type = EV_KEY;
    ie.code = BTN_SELECT;
    ie.value = this->buttons.select;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->buttons.start != g.buttons.start)
  {
    this->buttons.start = g.buttons.start;
    ie.type = EV_KEY;
    ie.code = BTN_START;
    ie.value = this->buttons.start;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->buttons.mode != g.buttons.mode)
  {
    this->buttons.mode = g.buttons.mode;
    ie.type = EV_KEY;
    ie.code = BTN_MODE;
    ie.value = this->buttons.mode;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->analogs.x != g.analogs.x)
  {
    this->analogs.x = g.analogs.x;
    ie.type = EV_ABS;
    ie.code = ABS_X;
    ie.value = this->analogs.x;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->analogs.y != g.analogs.y)
  {
    this->analogs.y = g.analogs.y;
    ie.type = EV_ABS;
    ie.code = ABS_Y;
    ie.value = this->analogs.y;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->analogs.z != g.analogs.z)
  {
    this->analogs.z = g.analogs.z;
    ie.type = EV_ABS;
    ie.code = ABS_Z;
    ie.value = this->analogs.z;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->analogs.rx != g.analogs.rx)
  {
    this->analogs.rx = g.analogs.rx;
    ie.type = EV_ABS;
    ie.code = ABS_RX;
    ie.value = this->analogs.rx;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->analogs.ry != g.analogs.ry)
  {
    this->analogs.ry = g.analogs.ry;
    ie.type = EV_ABS;
    ie.code = ABS_RY;
    ie.value = this->analogs.ry;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->analogs.rz != g.analogs.rz)
  {
    this->analogs.rz = g.analogs.rz;
    ie.type = EV_ABS;
    ie.code = ABS_RZ;
    ie.value = this->analogs.rz;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->analogs.hat0x != g.analogs.hat0x)
  {
    this->analogs.hat0x = g.analogs.hat0x;
    ie.type = EV_ABS;
    ie.code = ABS_HAT0Y;
    ie.value = this->analogs.hat0x;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
  if (this->analogs.hat0y != g.analogs.hat0y)
  {
    this->analogs.hat0y = g.analogs.hat0y;
    ie.type = EV_ABS;
    ie.code = ABS_HAT0X;
    ie.value = this->analogs.hat0y;
    if (write(uinput, &ie, sizeof(ie)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
    if (write(uinput, &syn, sizeof(syn)) < 0)
    {
      perror("ssbml::gamepad_spoofer::update(): write()");
    }
  }
}

ssbml::gamepad_spoofer::gamepad_spoofer() :
  uinput(open("/dev/uinput", O_WRONLY | O_NONBLOCK)),
  syn({
    .type = EV_SYN,
    .code = 0,
    .value = 0,
    .time = {.tv_sec = 0, .tv_usec = 0}
  })
{
  if (uinput < 0)
  {
    perror("/dev/uinput");
  }
  ioctl(uinput, UI_SET_EVBIT, EV_KEY);
  ioctl(uinput, UI_SET_EVBIT, EV_ABS);
  ioctl(uinput, UI_SET_KEYBIT, BTN_A);
  ioctl(uinput, UI_SET_KEYBIT, BTN_B);
  ioctl(uinput, UI_SET_KEYBIT, BTN_X);
  ioctl(uinput, UI_SET_KEYBIT, BTN_Y);
  ioctl(uinput, UI_SET_KEYBIT, BTN_TR);
  ioctl(uinput, UI_SET_KEYBIT, BTN_TL);
  ioctl(uinput, UI_SET_KEYBIT, BTN_THUMBL);
  ioctl(uinput, UI_SET_KEYBIT, BTN_THUMBR);
  ioctl(uinput, UI_SET_KEYBIT, BTN_SELECT);
  ioctl(uinput, UI_SET_KEYBIT, BTN_START);
  ioctl(uinput, UI_SET_KEYBIT, BTN_MODE);

  struct uinput_user_dev udev;
  memset(&udev, 0, sizeof(udev));
  strcpy(udev.name, "Spoofed Xbox Controller");
  udev.id.bustype = BUS_USB;
  udev.id.vendor = 0x0420;
  udev.id.product = 0x6969;
  udev.absmin[ABS_X] = -32768;
  udev.absmax[ABS_X] = 32767;
  udev.absfuzz[ABS_X] = 16;
  udev.absflat[ABS_X] = 128;
  udev.absmin[ABS_Y] = -32768;
  udev.absmax[ABS_Y] = 32767;
  udev.absfuzz[ABS_Y] = 16;
  udev.absflat[ABS_Y] = 128;
  udev.absmin[ABS_Z] = 0;
  udev.absmax[ABS_Z] = 1023;
  udev.absmin[ABS_RX] = -32768;
  udev.absmax[ABS_RX] = 32767;
  udev.absfuzz[ABS_RX] = 16;
  udev.absflat[ABS_RX] = 128;
  udev.absmin[ABS_RY] = -32768;
  udev.absmax[ABS_RY] = 32767;
  udev.absfuzz[ABS_RY] = 16;
  udev.absflat[ABS_RY] = 128;
  udev.absmin[ABS_RZ] = 0;
  udev.absmax[ABS_RZ] = 1023;
  udev.absmin[ABS_HAT0X] = -1;
  udev.absmax[ABS_HAT0X] = 1;
  udev.absmin[ABS_HAT0Y] = -1;
  udev.absmax[ABS_HAT0Y] = 1;

  if (write(uinput, &udev, sizeof(udev)) < 0)
  {
    perror("ssbml::gamepad_spoofer::gamepad_spoofer(): write()");
  }

  ioctl(uinput, UI_SET_ABSBIT, ABS_X);
  ioctl(uinput, UI_SET_ABSBIT, ABS_Y);
  ioctl(uinput, UI_SET_ABSBIT, ABS_RX);
  ioctl(uinput, UI_SET_ABSBIT, ABS_RY);
  ioctl(uinput, UI_SET_ABSBIT, ABS_RZ);
  ioctl(uinput, UI_SET_ABSBIT, ABS_Z);
  ioctl(uinput, UI_SET_ABSBIT, ABS_HAT0X);
  ioctl(uinput, UI_SET_ABSBIT, ABS_HAT0Y);

  struct uinput_setup usetup;
  memset(&usetup, 0, sizeof(usetup));
  strcpy(usetup.name, "Spoofed Xbox Controller");
  usetup.id.bustype = BUS_USB;
  usetup.id.vendor = 0x0420;
  usetup.id.product = 0x6969;
  ioctl(uinput, UI_DEV_SETUP, &usetup);
  ioctl(uinput, UI_DEV_CREATE);
}

ssbml::gamepad_spoofer::~gamepad_spoofer()
{
  ioctl(uinput, UI_DEV_DESTROY);
  close(uinput);
}
