#include "test_window.h"

void ssbml::gui::test_window::get_test_info()
{
  struct test_session::info info;

  testSession.get_info(info);
  gamepadVisualizer << info.c;
  if (info.modelLoaded)
  {
    label.set_text(info.testing ? "TESTING" : "NOT TESTING");
    gamepadVisualizer.set_visible(true);
  }
  else
  {
    label.set_text("Loading model...");
    gamepadVisualizer.set_visible(false);
  }
}

ssbml::gui::test_window::test_window(Display *display, ::Window window,
  uint64_t frameWidth, uint64_t frameHeight, double fps,
  std::string gamepadDeviceFileName, std::string modelMeta,
  gamepad_spoofer &gamepadSpoofer) :
  label("Loading model..."),
  box(Gtk::ORIENTATION_VERTICAL, 10),
  gamepadVisualizer(256),
  testSession(display, window, frameWidth, frameHeight, fps,
    gamepadDeviceFileName, modelMeta, gamepadSpoofer, dispatcher)
{
  set_title("Test Session");
  set_border_width(20);
  set_default_size(600, 300);
  set_modal(true);

  box.add(label);
  gamepadVisualizer.set_halign(Gtk::ALIGN_CENTER);
  box.add(gamepadVisualizer);
  add(box);

  dispatcher.connect(sigc::mem_fun(*this, &test_window::get_test_info));

  show_all_children();
}
