#include "RecordWindow.h"

void SSBML::RecordWindow::on_hide_callback()
{
  recordSession.end();
}

SSBML::RecordWindow::RecordWindow(Display *display, ::Window window,
  std::string gamepadDeviceFileName, std::string destinationFolder) :
  display(display), window(window),
  gamepadDeviceFileName(gamepadDeviceFileName),
  destinationFolder(destinationFolder),
  recordSession(display, window, gamepadDeviceFileName, destinationFolder + "/")
{
  set_title("Record");
  set_border_width(0);
  set_default_size(600, 300);
  set_modal(true);

  signal_hide().connect(sigc::mem_fun(*this, &RecordWindow::on_hide_callback));

  recordSession.init();

  show_all_children();
}
