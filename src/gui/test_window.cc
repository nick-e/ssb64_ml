#include "test_window.h"

void ssbml::gui::test_window::get_test_info()
{
  bool testing;
  bool modelLoaded;

  testSession.get_test_info(&testing, &modelLoaded);
  if (modelLoaded)
  {
    label.set_text(testing ? "TESTING" : "NOT TESTING");
  }
  else
  {
    label.set_text("Loading model...");
  }
}

ssbml::gui::test_window::test_window(Display *display, ::Window window,
  uint64_t frameWidth, uint64_t frameHeight, double fps,
  std::string gamepadDeviceFileName, std::string modelMeta,
  gamepad_spoofer &gamepadSpoofer) :
  label("Loading model..."),
  testSession(display, window, frameWidth, frameHeight, fps,
    gamepadDeviceFileName, modelMeta, gamepadSpoofer, dispatcher)
{
  set_title("Test Session");
  set_border_width(20);
  set_default_size(600, 300);
  set_modal(true);

  add(label);

  dispatcher.connect(sigc::mem_fun(*this, &test_window::get_test_info));

  show_all_children();
}
