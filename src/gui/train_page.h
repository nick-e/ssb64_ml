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

      Gtk::Label modelMetaLabel;
			Gtk::FileChooserButton modelMetaFileChooser;
			Gtk::Box modelMetaBox;
			Gtk::Button createModelButton;
			Gtk::Box modelBox;
			Gtk::Frame modelFrame;

			Gtk::Label trainDirectoryLabel;
			Gtk::FileChooserButton trainDirectoryChooser;
			Gtk::Box trainBox;
			Gtk::Label testDirectoryLabel;
			Gtk::FileChooserButton testDirectoryChooser;
			Gtk::Box testBox;
			Gtk::Box dataBox;
			Gtk::Frame dataFrame;

			Gtk::Label saveLossLabel;
			Gtk::CheckButton saveLossCheckButton;
			Gtk::Box lossSubBox1;
			Gtk::Label lossDirectoryLabel;
			Gtk::FileChooserButton lossDirectoryChooser;
			Gtk::Box lossSubBox2;
			Gtk::Label lossFileNameLabel;
			Gtk::Entry lossFileNameEntry;
			Gtk::Box lossSubBox3;
			Gtk::Box lossBox;
			Gtk::Frame lossFrame;

			Gtk::Label epochLabel;
			Gtk::SpinButton epochSpinButton;
			Gtk::Box epochBox;

			Gtk::Label lookbackLabel;
			Gtk::SpinButton lookbackSpinButton;
			Gtk::Box lookbackBox;

			Gtk::Label downsampleLabel;
			Gtk::SpinButton downsampleSpinButton;
			Gtk::Box downsampleBox;

			Gtk::Label suspendLabel;
			Gtk::CheckButton suspendCheckButton;
			Gtk::Box suspendBox;

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
