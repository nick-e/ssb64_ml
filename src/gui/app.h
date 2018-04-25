#ifndef APP_H
#define APP_H

#include <gtkmm-3.0/gtkmm.h>

#include "record_page.h"
#include "train_page.h"
#include "test_page.h"

namespace ssbml
{
  namespace gui
  {
    class app : public Gtk::Window
    {
    public:
      app();

    protected:
      Gtk::Notebook notebook;
      record_page record;
      train_page train;
      test_page test;
    };
  }
}

#endif
