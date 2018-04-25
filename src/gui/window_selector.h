#ifndef WINDOW_SELECTOR_H
#define WINDOW_SELECTOR_H

#include <string>

#include <gtkmm-3.0/gtkmm.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#include "window_selector_popup.h"

namespace ssbml
{
  namespace gui
  {
    class window_selector : public Gtk::Frame
    {
    public:
      window_selector(std::string title, Gtk::Window &mainWindow);

      ::Window get_window();
      Display* get_display();
    protected:
      Gtk::Window &mainWindow;
      Gtk::Box box;
      Gtk::TreeView treeView;
      window_selector_popup::ModelColumns modelColumns;
      Glib::RefPtr<Gtk::ListStore> refTreeModel;
      Gtk::Button button;
      Display *display;
      ::Window selectedWindow;
      window_selector_popup *windowSelectorPopup;

      void on_button_clicked();
      void on_window_selector_popup_row_activated(const Gtk::TreeModel::Path& path,
        Gtk::TreeViewColumn* column);
      void on_window_selector_popup_select_button_clicked();
      void get_selected_window();
    };
  }
}

#endif
