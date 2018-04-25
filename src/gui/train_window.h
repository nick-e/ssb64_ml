#ifndef TRAIN_WINDOW_H
#define TRAIN_WINDOW_H

#include <gtkmm-3.0/gtkmm.h>

#include "../core/train_session.h"

namespace ssbml
{
  namespace gui
  {
    class train_window : public Gtk::Window
    {
    public:
      Gtk::ProgressBar progressBar;
      Gtk::Label modelLabel;
      Gtk::Label epochLabel;
      Gtk::Label fileLabel;
      Gtk::Label frameLabel;
      Gtk::Label fileNameLabel;
      Gtk::Box infoBox;
      Gtk::Box box;
      Glib::Dispatcher dispatcher;

      train_window(std::string modelDir, std::string trainingDataDir,
        uint64_t epochs, uint64_t batchSize);

    protected:
      uint64_t epochs;
      uint64_t batchSize;
      train_session trainSession;

      void on_create_model_button_clicked();
      void get_train_info();
    };
  }
}

#endif
