#ifndef GAMEPAD_SELECTOR_H
#define GAMEPAD_SELECTOR_H

#include <string>

#include <gtkmm-3.0/gtkmm.h>

#include "gamepad_selector_popup.h"

namespace ssbml
{
  namespace gui
  {
    class gamepad_selector : public Gtk::Frame
    {
    public:
      gamepad_selector(std::string title, Gtk::Window &mainWindow);

      std::string get_selected_gamepad_device_file_name();

    protected:
      Gtk::Window &mainWindow;
      Gtk::Box box;
      Gtk::TreeView treeView;
      gamepad_selector_popup::ModelColumns modelColumns;
      Glib::RefPtr<Gtk::ListStore> refTreeModel;
      Gtk::Button button;
      gamepad_selector_popup *gamepadSelectorPopup;
      std::string selectedGamepadDeviceFileName;

      void get_selected_gamepad();
      void on_button_clicked();
      void on_gamepad_selector_popup_select_button_clicked();
      void on_gamepad_selector_popup_row_activated(
        const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
    };
  }
}

#endif
