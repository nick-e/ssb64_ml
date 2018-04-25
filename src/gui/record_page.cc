#include "record_page.h"

#include <iostream>

bool ssbml::gui::record_page::on_record_window_closed(GdkEventAny *event)
{
  delete recordWindow;
  return false;
}

void ssbml::gui::record_page::on_startButton_clicked()
{
  ::Window window = windowSelector.get_window();
  Display *display = windowSelector.get_display();
  std::string gamepadDeviceFileName = gamepadSelector.get_selected_gamepad_device_file_name();
  std::string dir = dd_fileChooser.get_filename();

  if (window == -1)
  {
    Gtk::MessageDialog dialog(mainWindow, "Missing Window To Record",
      false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    dialog.set_secondary_text("You must select a window to record");
    dialog.run();
  }
  else if (gamepadDeviceFileName.empty())
  {
    Gtk::MessageDialog dialog(mainWindow, "Missing Gamepad To Record",
      false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    dialog.set_secondary_text("You must select a gamepad to record");
    dialog.run();
  }
  else if (dir.empty())
  {
    Gtk::MessageDialog dialog(mainWindow, "Missing Destination Directory",
      false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    dialog.set_secondary_text("You must select a directory for where to " \
      "save the recording data");
    dialog.run();
  }
  else
  {
    recordWindow = new ssbml::gui::record_window(display, window,
      gamepadDeviceFileName, dir);
    recordWindow->signal_delete_event().connect(sigc::mem_fun(*this,
      &record_page::on_record_window_closed));
    recordWindow->set_transient_for(mainWindow);
    recordWindow->show();
  }
}

ssbml::gui::record_page::record_page(Gtk::Window &mainWindow) :
  Box(Gtk::ORIENTATION_VERTICAL, 10),
  mainWindow(mainWindow),
  windowSelector("Window To Record", mainWindow),
  gamepadSelector("Gamepad To Record", mainWindow),
  startButton("Start"),
  dd_box(Gtk::ORIENTATION_HORIZONTAL),
  dd_label("Destination Directory:\t"),
  dd_fileChooser(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER)
{
  set_border_width(20);

  add(windowSelector);
  add(gamepadSelector);

  dd_box.add(dd_label);
  dd_box.add(dd_fileChooser);
  add(dd_box);

  startButton.set_halign(Gtk::ALIGN_END);
  startButton.set_valign(Gtk::ALIGN_END);
  startButton.signal_clicked().connect(sigc::mem_fun(*this,
    &record_page::on_startButton_clicked));
  pack_end(startButton);

  show_all_children();
}
