#include "window_selector.h"

Display* ssbml::gui::window_selector::get_display()
{
  return display;
}

Window ssbml::gui::window_selector::get_window()
{
  return selectedWindow;
}

static bool select_function(const Glib::RefPtr<Gtk::TreeModel>& model,
  const Gtk::TreeModel::Path& path, bool pathCurrentlySelected)
{
  return false;
}

void ssbml::gui::window_selector::get_selected_window()
{
  display = windowSelectorPopup->get_selected_window(&selectedWindow);
  refTreeModel->clear();
  Gtk::TreeModel::Row row = *(refTreeModel->append());
  uint32_t pid = video_input::get_window_pid(display, selectedWindow);
  row[modelColumns.windowTitle] = video_input::get_window_title(display,
    selectedWindow);
  row[modelColumns.processName] = video_input::get_process_name(pid);
  row[modelColumns.pid] = std::to_string(pid);
  row[modelColumns.index] = 0;
  windowSelectorPopup->close();
}

void ssbml::gui::window_selector::on_window_selector_popup_row_activated(
  const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column)
{
  get_selected_window();
}

void ssbml::gui::window_selector::on_window_selector_popup_select_button_clicked()
{
  get_selected_window();
}

void ssbml::gui::window_selector::on_button_clicked()
{
  windowSelectorPopup = new ssbml::gui::window_selector_popup();
  windowSelectorPopup->selectButton.signal_clicked().connect(sigc::mem_fun(
    *this, &window_selector::on_window_selector_popup_select_button_clicked));
  windowSelectorPopup->treeView.signal_row_activated().connect(sigc::mem_fun(
    *this, &window_selector::on_window_selector_popup_row_activated));
  windowSelectorPopup->set_transient_for(mainWindow);
  windowSelectorPopup->show();
}

ssbml::gui::window_selector::window_selector(std::string title,
  Gtk::Window &mainWindow) :
  Gtk::Frame(title),
  mainWindow(mainWindow),
  box(Gtk::ORIENTATION_VERTICAL, 10),
  button("Change"),
  display(nullptr),
  selectedWindow(-1),
  windowSelectorPopup(nullptr)
{
  refTreeModel = Gtk::ListStore::create(modelColumns);
  refTreeModel->append();
  treeView.set_model(refTreeModel);
  treeView.append_column("PID", modelColumns.pid);
  treeView.append_column("Process Name", modelColumns.processName);
  treeView.append_column("Window Title", modelColumns.windowTitle);
  Glib::RefPtr<Gtk::TreeSelection> treeSelection = treeView.get_selection();
  treeSelection->set_select_function(&select_function);
  box.add(treeView);

  button.signal_clicked().connect(sigc::mem_fun(*this,
    &window_selector::on_button_clicked));
  button.set_halign(Gtk::ALIGN_START);
  box.add(button);

  box.set_border_width(10);
  add(box);

  show_all_children();
}
