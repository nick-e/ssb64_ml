#include "record_window.h"

static std::string time_to_string(unsigned long time)
{
  unsigned long x = time / 100000;
  unsigned long dec = x % 10;
  x /= 10;
  unsigned long seconds = x % 60;
  x /= 60;
  unsigned long minutes = x % 60;
  x /= 60;
  unsigned long hourecordSession = x % 24;

  std::string str = "";
  if (hourecordSession < 10)
  {
    str += "0";
  }
  str += std::to_string(hourecordSession) + ":";
  if (minutes < 10)
  {
    str += "0";
  }
  str += std::to_string(minutes) + ":";
  if (seconds < 10)
  {
    str += "0";
  }
  str += std::to_string(seconds) + "." + std::to_string(dec);

  return str;
}

void ssbml::gui::record_window::get_record_info()
{
  bool recording;
  unsigned long totalTime;
  std::string filePrefix;

  recordSession.get_record_info(&recording, &totalTime, filePrefix);
  recordingLabel.set_text(recording ? "RECORDING" : "NOT RECORDING");
  timeLabel.set_text(time_to_string(totalTime));
  prefixLabel.set_text(filePrefix);
}

ssbml::gui::record_window::record_window(Display *display, ::Window window,
  std::string gamepadDeviceFileName, std::string dstFolder) :
  recordingLabel("NOT RECORDING"),
  timeLabel("00:00:00.0"),
  prefixLabel(""),
  box(Gtk::ORIENTATION_VERTICAL, 10),
  recordSession(display, window, 256, 144, 30.0, gamepadDeviceFileName,
    dstFolder + "/", dispatcher)
{
  set_title("Record Session");
  set_border_width(20);
  set_default_size(600, 300);
  set_modal(true);

  box.add(recordingLabel);
  box.add(timeLabel);
  box.add(prefixLabel);
  add(box);

  dispatcher.connect(sigc::mem_fun(*this, &record_window::get_record_info));

  show_all_children();
}
