#ifndef TRAIN_PAGE_H
#define TRAIN_PAGE_H

#include "train_window.h"

namespace ssbml
{
  namespace gui
  {
    class train_page : public Gtk::Box
    {
    public:
      train_page(Gtk::Window &mainWindow);

    protected:
      Gtk::Window &mainWindow;
      Gtk::Box model_box;
      Gtk::Label model_label1;
      Gtk::FileChooserButton model_fileChooser;
      Gtk::Label model_label2;
      Gtk::Button model_button;
      Gtk::Box epoch_box;
      Gtk::Label epoch_label;
      Gtk::SpinButton epoch_spinButton;
      Gtk::Box batch_box;
      Gtk::Label batch_label;
      Gtk::SpinButton batch_spinButton;
      Gtk::Frame data_frame;
      Gtk::Box data_box;
      Gtk::Box train_box;
      Gtk::Label train_label;
      Gtk::FileChooserButton train_fileChooser;
      Gtk::Box test_box;
      Gtk::Label test_label;
      Gtk::FileChooserButton test_fileChooser;
      Gtk::Button trainButton;
      train_window *trainWindow;

      void on_create_model_button_clicked();
      void on_train_button_clicked();
      bool on_train_window_closed(GdkEventAny *event);
      void on_startButton_clicked();
    };
  }
}

#endif
