#ifndef APP_H
#define APP_H

#include <gtkmm-3.0/gtkmm/window.h>
#include <gtkmm-3.0/gtkmm/notebook.h>
#include <gtkmm-3.0/gtkmm/label.h>
#include <gtkmm-3.0/gtkmm/button.h>
#include <gtkmm-3.0/gtkmm/filechooserbutton.h>
#include <gtkmm-3.0/gtkmm/filechooserdialog.h>
#include <gtkmm-3.0/gtkmm/frame.h>
#include <gtkmm-3.0/gtkmm/treemodel.h>
#include <gtkmm-3.0/gtkmm/liststore.h>
#include <gtkmm-3.0/gtkmm/treeview.h>
#include <gtkmm-3.0/gtkmm/spinbutton.h>
#include <gtkmm-3.0/gtkmm/adjustment.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#include "VideoInputSelector.h"
#include "GamepadSelector.h"
#include "RecordWindow.h"
#include "TrainWindow.h"

#define SSBML_APPLICATION_ID "com.nickscotteaton.ssbml"

namespace SSBML
{
  class App : public Gtk::Window
  {
  public:
    App();

  protected:
    class TrainPage : public Gtk::Box
    {
    public:
      TrainPage(App &app);

    protected:
      App &app;
      Gtk::Frame model_frame;
      Gtk::Box model_box;
      Gtk::Label model_label1;
      Gtk::FileChooserButton model_fileChooser;
      Gtk::Label model_label2;
      Gtk::Button model_button;
      Gtk::Box epoch_box;
      Gtk::Label epoch_label;
      Gtk::SpinButton epoch_spinButton;
      Gtk::Frame data_frame;
      Gtk::Box data_box;
      Gtk::Box train_box;
      Gtk::Label train_label;
      Gtk::FileChooserButton train_fileChooser;
      Gtk::Box test_box;
      Gtk::Label test_label;
      Gtk::FileChooserButton test_fileChooser;
      Gtk::Button trainButton;
      TrainWindow *trainWindow;

      void on_create_model_button_clicked();
      void on_train_button_clicked();
    };

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
      Gtk::Button vis_button;
      Gtk::Frame gs_frame;
      Gtk::Box gs_box;
      Gtk::TreeView gs_treeView;
      GamepadSelector::ModelColumns gs_modelColumns;
      Glib::RefPtr<Gtk::ListStore> gs_refTreeModel;
      Gtk::Button gs_button;
      Gtk::Button startButton;
      Gtk::Box dd_box;
      Gtk::Label dd_label;
      Gtk::FileChooserButton dd_fileChooser;
      Display *display;
      ::Window selectedWindow;
      std::string selectedGamepadDeviceFileName;
      VideoInputSelector *videoInputSelector;
      GamepadSelector *gamepadSelector;
      RecordWindow *recordWindow;

      void get_selected_window();
      void get_selected_gamepad();
      void on_change_window_button_clicked();
      void on_change_gamepad_button_clicked();
      void on_videoInputSelector_select_button_clicked();
      void on_videoInputSelector_row_activated(
        const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
      void on_gamepadSelector_select_button_clicked();
      void on_gamepadSelector_row_activated(const Gtk::TreeModel::Path& path,
        Gtk::TreeViewColumn* column);
      void on_startButton_clicked();
    };

    Gtk::Notebook notebook;
    RecordPage recordPage;
    TrainPage trainPage;
  };
}

#endif
