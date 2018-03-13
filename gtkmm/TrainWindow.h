#ifndef TRAINWINDOW_H
#define TRAINWINDOW_H

#include <gtkmm-3.0/gtkmm/window.h>

#include "../core/NNModel.h"

namespace SSBML
{
  class TrainWindow : public Gtk::Window
  {
  public:
    TrainWindow();

  protected:
    void on_create_model_button_clicked();
  };
}

#endif
