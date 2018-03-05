#include "RecordPage.h"

#include <iostream>

void SSBML::App::RecordPage::get_selected_window()
{
  display = videoInputSelector->get_selected_window(&recordWindow);
  vis_refTreeModel->clear();
  Gtk::TreeModel::Row row = *(vis_refTreeModel->append());
  char *windowTitle;
  uint32_t pid = VideoInput::get_window_pid(display, recordWindow);
  row[vis_modelColumns.pid] = std::to_string(pid);
  if (!XFetchName(display, recordWindow, &windowTitle))
  {
    row[vis_modelColumns.windowTitle] = "";
  }
  else
  {
    row[vis_modelColumns.windowTitle] = windowTitle;
  }
  std::ifstream cmdline("/proc/" + std::to_string(pid) + "/cmdline");
  std::string processName;
  std::getline(cmdline, processName);
  row[vis_modelColumns.processName] = processName;
  row[vis_modelColumns.index] = 0;
  cmdline.close();
  videoInputSelector->close();
}

void SSBML::App::RecordPage::on_videoInputSelector_select_button_clicked()
{
  get_selected_window();
}

void SSBML::App::RecordPage::on_videoInputSelector_row_activated(
  const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column)
{
  get_selected_window();
}

void SSBML::App::RecordPage::on_gamepadSelector_select_button_clicked()
{
  get_selected_gamepad();
}

void SSBML::App::RecordPage::on_change_window_button_clicked()
{
  videoInputSelector = new SSBML::VideoInputSelector();
  videoInputSelector->selectButton.signal_clicked().connect(sigc::mem_fun(
    *this, &RecordPage::on_videoInputSelector_select_button_clicked));
  videoInputSelector->treeView.signal_row_activated().connect(sigc::mem_fun(
    *this, &RecordPage::on_videoInputSelector_row_activated));
  videoInputSelector->set_transient_for((Gtk::Window&)app);
  videoInputSelector->show();
}

void SSBML::App::RecordPage::get_selected_gamepad()
{
  gamepadSelector->close();
}

void SSBML::App::RecordPage::on_gamepadSelector_row_activated(
  const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column)
{
  get_selected_gamepad();
}

void SSBML::App::RecordPage::on_change_gamepad_button_clicked()
{
  gamepadSelector = new SSBML::GamepadSelector();
  gamepadSelector->selectButton.signal_clicked().connect(sigc::mem_fun(
    *this, &RecordPage::on_gamepadSelector_select_button_clicked));
  gamepadSelector->treeView.signal_row_activated().connect(sigc::mem_fun(
    *this, &RecordPage::on_gamepadSelector_row_activated));
  gamepadSelector->set_transient_for((Gtk::Window&)app);
  gamepadSelector->show();
}

static bool select_function(const Glib::RefPtr<Gtk::TreeModel>& model,
  const Gtk::TreeModel::Path& path, bool pathCurrentlySelected)
{
  return false;
}

SSBML::App::RecordPage::RecordPage(App &app) :
  Box(Gtk::ORIENTATION_VERTICAL),
  app(app),
  vis_frame("Window To Record"),
  vis_box(Gtk::ORIENTATION_VERTICAL, 10),
  vis_button("Change"),
  gs_frame("Gamepad To Record"),
  gs_box(Gtk::ORIENTATION_VERTICAL, 10),
  gs_button("Change"),
  startButton("Start"),
  dstDirChooser(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
  display(NULL),
  videoInputSelector(NULL),
  gamepadSelector(NULL),
  gs_Fd(-1)
{
  set_border_width(20);

  vis_refTreeModel = Gtk::ListStore::create(vis_modelColumns);
  vis_treeView.set_model(vis_refTreeModel);
  vis_treeView.append_column("PID", vis_modelColumns.pid);
  vis_treeView.append_column("Process Name", vis_modelColumns.processName);
  vis_treeView.append_column("Window Title", vis_modelColumns.windowTitle);
  vis_row = *(vis_refTreeModel->append());
  Glib::RefPtr<Gtk::TreeSelection> treeSelection = vis_treeView.get_selection();
  treeSelection->set_select_function(&select_function);
  vis_box.add(vis_treeView);

  vis_button.signal_clicked().connect(sigc::mem_fun(*this,
    &RecordPage::on_change_window_button_clicked));
  vis_button.set_halign(Gtk::ALIGN_START);
  vis_box.add(vis_button);

  vis_box.set_border_width(10);
  vis_frame.add(vis_box);
  add(vis_frame);

  gs_button.signal_clicked().connect(sigc::mem_fun(*this,
    &RecordPage::on_change_gamepad_button_clicked));
  gs_button.set_halign(Gtk::ALIGN_START);
  gs_box.add(gs_button);

  gs_box.set_border_width(10);
  gs_frame.add(gs_box);
  add(gs_frame);

  add(dstDirChooser);

  startButton.set_halign(Gtk::ALIGN_END);
  add(startButton);

  show_all_children();
}
