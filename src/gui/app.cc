#include "app.h"

ssbml::gui::app::app() : record(*this), train(*this), test(*this)
{
  set_title("SSBML");
  set_border_width(0);
  set_default_size(800, 500);

  notebook.set_border_width(10);
  add(notebook);

  notebook.append_page(record, "record");
  notebook.append_page(train, "train");
  notebook.append_page(test, "test");

  show_all_children();
}

int main(int argc, char *argv[])
{
  av_register_all();
  av_log_set_level(AV_LOG_QUIET);
  auto app = Gtk::Application::create(argc, argv, "com.nickscotteaton.ssbml");
  ssbml::gui::app ssbmlApp;

  return app->run(ssbmlApp);
}
