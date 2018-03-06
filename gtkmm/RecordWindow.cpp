#include "RecordWindow.h"

#include <iostream>

static void* thread_routine(void *arg)
{
  SSBML::RecordWindow *recordWindow = (SSBML::RecordWindow*)arg;
  recordWindow->gamepad.init();
  while (!recordWindow->quit);
  {

  }
  recordWindow->gamepad.end();
  return NULL;
}

void SSBML::RecordWindow::on_hide_callback()
{
  quit = true;
  if (pthread_join(tid, NULL) < 0)
  {
    perror("SSBML::RecordWindow::RecordWindow(): pthread_join()");
  }
}

SSBML::RecordWindow::RecordWindow(Display *display, ::Window window,
  std::string gamepadDeviceFileName, std::string destinationFolder) :
  display(display), window(window), gamepad(gamepadDeviceFileName),
  videoInput(display, window, 256, 144), destinationFolder(destinationFolder),
  quit(false)
{
  set_title("Record");
  set_border_width(0);
  set_default_size(600, 300);
  set_modal(true);

  signal_hide().connect(sigc::mem_fun(*this, &RecordWindow::on_hide_callback));

  if (pthread_create(&tid, NULL, thread_routine, this) < 0)
  {
    perror("SSBML::RecordWindow::RecordWindow(): pthread_create()");
  }

  show_all_children();
}
