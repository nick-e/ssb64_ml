#ifndef VIDEOINPUTSELECTOR_H
#define VIDEOINPUTSELECTOR_H

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

#include "../core/VideoInput.h"

namespace SSBML
{
    class VideoInputSelector : public Gtk::Window
    {
    public:
      class ModelColumns : public Gtk::TreeModel::ColumnRecord
      {
      public:
        Gtk::TreeModelColumn<Glib::ustring> pid;
        Gtk::TreeModelColumn<Glib::ustring> processName;
        Gtk::TreeModelColumn<Glib::ustring> windowTitle;
        Gtk::TreeModelColumn<unsigned long> index;

        ModelColumns();
      };

      Gtk::Button selectButton;
      Gtk::TreeView treeView;

      VideoInputSelector();
      ~VideoInputSelector();
      Display* get_selected_window(::Window *window);

    protected:

      Gtk::Box box;
      Gtk::ButtonBox buttonBox;
      Gtk::Button refreshButton;
      ModelColumns modelColumns;
      Glib::RefPtr<Gtk::ListStore> refTreeModel;
      Gtk::ScrolledWindow scrolledWindow;
      Display *display;
      ::Window *windows;
      unsigned long numWindows;

      void refresh();
      bool on_key_press_event(GdkEventKey *event);
    };
}

#endif
