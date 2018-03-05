#ifndef GAMEPADSELECTOR_H
#define GAMEPADSELECTOR_H

#include <gtkmm-3.0/gtkmm/window.h>
#include <gtkmm-3.0/gtkmm/treeview.h>
#include <gtkmm-3.0/gtkmm/scrolledwindow.h>
#include <gtkmm-3.0/gtkmm/button.h>
#include <gtkmm-3.0/gtkmm/treemodel.h>
#include <gtkmm-3.0/gtkmm/liststore.h>
#include <gtkmm-3.0/gtkmm/box.h>
#include <gtkmm-3.0/gtkmm/buttonbox.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <fstream>
#include <string>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <fcntl.h>

#include "../core/Gamepad.h"

namespace SSBML
{
    class GamepadSelector : public Gtk::Window
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

      GamepadSelector();
      std::string get_selected_gamepad();

    protected:

      Gtk::Box box;
      Gtk::TreeView treeView;
      Gtk::ButtonBox buttonBox;
      Gtk::Button refreshButton;
      ModelColumns modelColumns;
      Glib::RefPtr<Gtk::ListStore> refTreeModel;
      Gtk::ScrolledWindow scrolledWindow;
      std::string gamepadDeviceFileNames[1024];
      unsigned long numGamepads;

      void refresh();
      bool on_key_press_event(GdkEventKey *event);
    };
}

#endif
