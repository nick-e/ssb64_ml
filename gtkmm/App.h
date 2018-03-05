#ifndef APP_H
#define APP_H

#include <gtkmm-3.0/gtkmm/window.h>
#include <gtkmm-3.0/gtkmm/notebook.h>
#include <gtkmm-3.0/gtkmm/label.h>
#include <gtkmm-3.0/gtkmm/button.h>
#include <gtkmm-3.0/gtkmm/filechooserbutton.h>
#include <gtkmm-3.0/gtkmm/frame.h>
#include <gtkmm-3.0/gtkmm/treemodel.h>
#include <gtkmm-3.0/gtkmm/liststore.h>
#include <gtkmm-3.0/gtkmm/treeview.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#include "VideoInputSelector.h"
#include "GamepadSelector.h"

#define SSBML_APPLICATION_ID "com.nickscotteaton.ssbml"

namespace SSBML
{
  class App : public Gtk::Window
  {
  public:
    App();

  protected:
    class RecordPage : public Gtk::Box
    {
    public:
      RecordPage(App &app);

    protected:
      App &app;
      Gtk::Frame vis_frame;
      Gtk::Box vis_box;
      Gtk::TreeView vis_treeView;
      VideoInputSelector::ModelColumns vis_modelColumns;
      Glib::RefPtr<Gtk::ListStore> vis_refTreeModel;
      Gtk::TreeModel::Row vis_row;
      Gtk::Button vis_button;
      Gtk::Frame gs_frame;
      Gtk::Box gs_box;
      Gtk::Button gs_button;
      Gtk::Button startButton;
      Gtk::FileChooserButton dstDirChooser;
      Display *display;
      ::Window recordWindow;
      VideoInputSelector *videoInputSelector;
      GamepadSelector *gamepadSelector;
      int gs_Fd;

      void on_change_window_button_clicked();
      void on_change_gamepad_button_clicked();
      void on_videoInputSelector_select_button_clicked();
      void on_gamepadSelector_select_button_clicked();
    };

    Gtk::Notebook notebook;
    RecordPage recordPage;
  };
}

#endif
