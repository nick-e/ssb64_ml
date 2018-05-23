#ifndef TRAIN_WINDOW_H
#define TRAIN_WINDOW_H

#include <vector>

#include <gtkmm-3.0/gtkmm.h>

#include "../core/train_session.h"
#include "../core/util.h"

namespace ssbml
{
  namespace gui
  {
    class train_window : public Gtk::Window
    {
    public:
      Gtk::Label epochLabel;
      Gtk::ProgressBar totalProgressBar;
      Gtk::Box totalBox;

      Gtk::Label fileNameLabel;
      Gtk::Label fileLabel;
      Gtk::ProgressBar epochProgressBar;
      Gtk::Box epochBox;

      Gtk::Label frameLabel;
      Gtk::ProgressBar fileProgressBar;
      Gtk::Box fileBox;

      Gtk::Label ttLabel;
      Gtk::Label ttLabel2;
      Gtk::Box ttBox;

      Gtk::Label etaLabel;
      Gtk::Label etaLabel2;
      Gtk::Box etaBox;

      Gtk::Label modelLabel;
			Gtk::Label modelLabel2;
      Gtk::ProgressBar modelProgressBar;
      Gtk::Box modelBox;

      Gtk::Box infoBox;
      Gtk::Box box;
      Glib::Dispatcher dispatcher;

			std::vector<double> losses;
      train_window(bool suspendOnCompletion, uint64_t downsampleRate,
				uint64_t lookback, uint64_t totalEpochs, std::string metaFile,
        std::string trainingDataDir);

    protected:
			uint64_t totalEpochs;
      train_session trainSession;

      void on_create_model_button_clicked();
      void get_train_info();
    };
  }
}

#endif
