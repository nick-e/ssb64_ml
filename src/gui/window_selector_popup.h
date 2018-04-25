#ifndef WINDOW_SELECTOR_POPUP_H
#define WINDOW_SELECTOR_POPUP_H

#include <gtkmm-3.0/gtkmm.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <fstream>
#include <string>

#include "../core/video_input.h"

namespace ssbml
{
  namespace gui
  {
    class window_selector_popup : public Gtk::Window
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

      window_selector_popup();
      ~window_selector_popup();
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
}

#endif
