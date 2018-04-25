#include "window_selector_popup.h"

Display* ssbml::gui::window_selector_popup::get_selected_window(::Window *window)
{

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = treeView.get_selection();
  Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
  if (iter)
  {
    Gtk::TreeModel::Row row = *iter;
    *window = windows[row[modelColumns.index]];
    return this->display;
  }
  return NULL;
}

void ssbml::gui::window_selector_popup::refresh()
{
  if (display != NULL)
  {
    refTreeModel->clear();
  }
  windows = video_input::get_all_visible_windows(&display, &numWindows);

  for (unsigned long i = 0; i < numWindows; i++)
  {
    ::Window window = windows[i];
    Gtk::TreeModel::Row row = *(refTreeModel->append());
    uint32_t pid = video_input::get_window_pid(display, window);
    row[modelColumns.windowTitle] = video_input::get_window_title(display, window);
    row[modelColumns.processName] = video_input::get_process_name(pid);
    row[modelColumns.pid] = std::to_string(pid);
    row[modelColumns.index] = i;
  }
}

ssbml::gui::window_selector_popup::~window_selector_popup()
{
  if (windows != NULL)
  {
    windows = NULL;
    display = NULL;
  }
}

ssbml::gui::window_selector_popup::ModelColumns::ModelColumns()
{
  add(pid);
  add(processName);
  add(windowTitle);
  add(index);
}

bool ssbml::gui::window_selector_popup::on_key_press_event(GdkEventKey *event)
{
  switch (event->keyval)
  {
    case GDK_KEY_F5:
      refresh();
      break;
    case GDK_KEY_Escape:
      close();
      break;
  }
  return true;
}

ssbml::gui::window_selector_popup::window_selector_popup() : selectButton("Select"),
  box(Gtk::ORIENTATION_VERTICAL, 10), refreshButton("Refresh")
{
  set_title("Select A Window");
  set_border_width(0);
  set_default_size(600, 300);
  set_modal(true);

  refTreeModel = Gtk::ListStore::create(modelColumns);
  treeView.set_model(refTreeModel);

  refresh();

  treeView.append_column("PID", modelColumns.pid);
  treeView.append_column("Process Name", modelColumns.processName);
  treeView.append_column("Window Title", modelColumns.windowTitle);

  scrolledWindow.add(treeView);
  scrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  box.pack_start(scrolledWindow);

  refreshButton.signal_clicked().connect(sigc::mem_fun(*this,
    &window_selector_popup::refresh));
  refreshButton.set_margin_left(10);
  refreshButton.set_margin_bottom(10);
  buttonBox.pack_start(refreshButton, Gtk::PACK_SHRINK);

  selectButton.set_margin_right(10);
  selectButton.set_margin_bottom(10);
  buttonBox.pack_start(selectButton, Gtk::PACK_SHRINK);

  buttonBox.set_layout(Gtk::BUTTONBOX_EDGE);
  box.pack_start(buttonBox, Gtk::PACK_SHRINK);
  add(box);

  signal_key_press_event().connect(sigc::mem_fun(*this,
    &window_selector_popup::on_key_press_event));

  show_all_children();
}
