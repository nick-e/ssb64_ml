#include "record_window.h"

void ssbml::gui::record_window::get_record_info()
{
  struct record_session::info info;

  recordSession.get_info(info);
  recordingLabel.set_text(info.recording ? "RECORDING" : "NOT RECORDING");
  timeLabel.set_text(time_to_string(info.totalTime));
  prefixLabel.set_text(info.filePrefix);
  gamepadVisualizer << info.c;
}

ssbml::gui::record_window::record_window(Display *display, ::Window window,
  std::string gamepadDeviceFileName, double fps, std::string dstFolder) :
  recordingLabel("NOT RECORDING"),
  timeLabel("00:00:00.0"),
  prefixLabel(""),
  box(Gtk::ORIENTATION_VERTICAL, 10),
  gamepadVisualizer(256),
  recordSession(display, window, 128, 72, fps, gamepadDeviceFileName,
    dstFolder + "/", dispatcher)
{
  set_title("Record Session");
  set_border_width(20);
  set_default_size(600, 300);
  set_modal(true);

  box.add(recordingLabel);
  box.add(timeLabel);
  box.add(prefixLabel);

  gamepadVisualizer.set_halign(Gtk::ALIGN_CENTER);
  box.add(gamepadVisualizer);
  add(box);

  dispatcher.connect(sigc::mem_fun(*this, &record_window::get_record_info));

  show_all_children();
}
