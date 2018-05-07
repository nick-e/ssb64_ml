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
      Gtk::Frame model_frame;
      Gtk::Box model_box;
      Gtk::Box model_box2;
      Gtk::Label model_label;
      Gtk::FileChooserButton model_fileChooser;
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
      Gtk::Frame loss_frame;
      Gtk::Box loss_box;
      Gtk::Box loss_box2;
      Gtk::Box loss_box3;
      Gtk::Box loss_box4;
      Gtk::Label loss_label;
      Gtk::FileChooserButton loss_fileChooser;
      Gtk::Label loss_label2;
      Gtk::Label loss_label3;
      Gtk::CheckButton loss_button;
      Gtk::Entry loss_entry;
      Gtk::FileChooserButton csv_fileChooser;
      Gtk::Box csv_box;
      Gtk::Label suspend_label;
      Gtk::CheckButton suspend_button;
      Gtk::Box suspend_box;
      Gtk::Button trainButton;
      train_window *trainWindow;

      void on_create_model_button_clicked();
      void on_train_button_clicked();
      bool on_train_window_closed(GdkEventAny *event);
      void on_startButton_clicked();
      void on_loss_button_toggled();
    };
  }
}

#endif
