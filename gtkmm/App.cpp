#include "App.h"

SSBML::App::App() : recordPage(*this)
{
  set_title("SSBML");
  set_border_width(0);
  set_default_size(800, 500);

  notebook.set_border_width(10);
  add(notebook);

  notebook.append_page(recordPage, "record");

  show_all_children();
}

int main(int argc, char *argv[])
{
  auto app = Gtk::Application::create(argc, argv, SSBML_APPLICATION_ID);
  SSBML::App a;

  return app->run(a);
}
