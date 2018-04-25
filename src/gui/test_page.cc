#include "test_page.h"

bool ssbml::gui::test_page::on_test_window_closed(GdkEventAny *event)
{
  delete testWindow;
  return false;
}

void ssbml::gui::test_page::on_test_button_clicked()
{
  ::Window window = windowSelector.get_window();
  Display *display = windowSelector.get_display();
  std::string gamepadDeviceFileName = gamepadSelector.get_selected_gamepad_device_file_name();
  std::string modelMeta = model_fileChooser.get_filename();

  if (window == -1)
  {
    Gtk::MessageDialog dialog(mainWindow, "Missing Window To Test",
      false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    dialog.set_secondary_text("You must select a window to test");
    dialog.run();
  }
  else if (gamepadDeviceFileName.empty())
  {
    Gtk::MessageDialog dialog(mainWindow, "Missing Gamepad To Listen To",
      false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    dialog.set_secondary_text("You must select a gamepad to listen to");
    dialog.run();
  }
  else if (modelMeta.empty())
  {
    Gtk::MessageDialog dialog(mainWindow, "Missing Model To Test",
      false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    dialog.set_secondary_text("You must select a .meta file");
    dialog.run();
  }
  else if (gamepadSpoofer == nullptr)
  {
    Gtk::MessageDialog dialog(mainWindow, "Missing Spoofed Gamepad",
      false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    dialog.set_secondary_text("You must create a spoofed gamepad");
    dialog.run();
  }
  else
  {
    testWindow = new ssbml::gui::test_window(display, window,
      256, 144, 30.0, gamepadDeviceFileName, modelMeta, *gamepadSpoofer);
    testWindow->signal_delete_event().connect(sigc::mem_fun(*this,
      &test_page::on_test_window_closed));
    testWindow->set_transient_for(mainWindow);
    testWindow->show();
  }
}

void ssbml::gui::test_page::test_page::create_gamepad_spoofer()
{
  if (gamepadSpoofer != nullptr)
  {
    delete gamepadSpoofer;
  }
  gamepadSpoofer = new gamepad_spoofer();
}

/*void ssbml::gui::test_page::test_page::test()
{
  ssbml::gamepad gamepad;
  gamepad.buttons.a = 1;
  gamepadSpoofer->update(gamepad);
}*/

ssbml::gui::test_page::test_page::test_page(Gtk::Window &mainWindow) :
  Box(Gtk::ORIENTATION_VERTICAL, 10),
  mainWindow(mainWindow),
  model_box(Gtk::ORIENTATION_HORIZONTAL, 10),
  model_label("Model To Test:\t"),
  windowSelector("Window To Test", mainWindow),
  gamepadSelector("Gamepad To Listen To", mainWindow),
  spoof_createButton("Create Gamepad Spoof"),
  testButton("Test"),
  gamepadSpoofer(nullptr),
  testWindow(nullptr)
{
  set_border_width(20);

  model_box.add(model_label);
  model_box.add(model_fileChooser);
  add(model_box);

  add(windowSelector);
  add(gamepadSelector);

  spoof_createButton.signal_clicked().connect(sigc::mem_fun(*this,
    &test_page::create_gamepad_spoofer));
  spoof_createButton.set_halign(Gtk::ALIGN_START);
  add(spoof_createButton);

  testButton.set_halign(Gtk::ALIGN_END);
  testButton.set_valign(Gtk::ALIGN_END);
  testButton.signal_clicked().connect(sigc::mem_fun(*this,
    &test_page::on_test_button_clicked));
  pack_end(testButton);

  show_all_children();
}

ssbml::gui::test_page::test_page::~test_page()
{
  if (gamepadSpoofer != nullptr)
  {
    delete gamepadSpoofer;
  }
}
