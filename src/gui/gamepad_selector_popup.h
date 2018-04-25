#ifndef GAMEPAD_SELECTOR_POPUP_H
#define GAMEPAD_SELECTOR_POPUP_H

#include <gtkmm-3.0/gtkmm.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <fstream>
#include <string>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <fcntl.h>

#include "../core/gamepad_listener.h"

namespace ssbml
{
  namespace gui
  {
    class gamepad_selector_popup : public Gtk::Window
    {
    public:
      class ModelColumns : public Gtk::TreeModel::ColumnRecord
      {
      public:
        Gtk::TreeModelColumn<Glib::ustring> deviceFileName;
        Gtk::TreeModelColumn<Glib::ustring> deviceName;
        Gtk::TreeModelColumn<unsigned long> index;

        ModelColumns();
      };

      Gtk::Button selectButton;
      Gtk::TreeView treeView;

      gamepad_selector_popup();
      std::string get_selected_gamepad();

    protected:

      Gtk::Box box;
      Gtk::ButtonBox buttonBox;
      Gtk::Button refreshButton;
      ModelColumns modelColumns;
      Glib::RefPtr<Gtk::ListStore> refTreeModel;
      Gtk::ScrolledWindow scrolledWindow;
      std::string gamepadDeviceFileNames[32];
      unsigned long numGamepads;

      void refresh();
      bool on_key_press_event(GdkEventKey *event);
    };
  }
}

#endif
