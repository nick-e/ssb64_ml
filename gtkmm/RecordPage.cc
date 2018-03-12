#include "RecordPage.h"

#include <iostream>

void SSBML::App::RecordPage::get_selected_window()
{
  display = videoInputSelector->get_selected_window(&selectedWindow);
  vis_refTreeModel->clear();
  Gtk::TreeModel::Row row = *(vis_refTreeModel->append());
  uint32_t pid = VideoInput::get_window_pid(display, selectedWindow);
  row[vis_modelColumns.windowTitle] = VideoInput::get_window_title(display,
    selectedWindow);
  row[vis_modelColumns.processName] = VideoInput::get_process_name(pid);
  row[vis_modelColumns.pid] = std::to_string(pid);
  row[vis_modelColumns.index] = 0;
  videoInputSelector->close();
}

void SSBML::App::RecordPage::get_selected_gamepad()
{
  selectedGamepadDeviceFileName = gamepadSelector->get_selected_gamepad();
  gs_refTreeModel->clear();
  Gtk::TreeModel::Row row = *(gs_refTreeModel->append());
  row[gs_modelColumns.deviceFileName] = selectedGamepadDeviceFileName;
  row[gs_modelColumns.deviceName] = Gamepad::get_device_name(
    selectedGamepadDeviceFileName);
  gamepadSelector->close();
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

void SSBML::App::RecordPage::on_startButton_clicked()
{
  recordWindow = new SSBML::RecordWindow(display, selectedWindow,
    selectedGamepadDeviceFileName, dd_fileChooser.get_filename());
  recordWindow->set_transient_for((Gtk::Window&)app);
  recordWindow->show();
}

static bool select_function(const Glib::RefPtr<Gtk::TreeModel>& model,
  const Gtk::TreeModel::Path& path, bool pathCurrentlySelected)
{
  return false;
}

SSBML::App::RecordPage::RecordPage(App &app) :
  Box(Gtk::ORIENTATION_VERTICAL, 10),
  app(app),
  vis_frame("Window To Record"),
  vis_box(Gtk::ORIENTATION_VERTICAL, 10),
  vis_button("Change"),
  gs_frame("Gamepad To Record"),
  gs_box(Gtk::ORIENTATION_VERTICAL, 10),
  gs_button("Change"),
  startButton("Start"),
  dd_box(Gtk::ORIENTATION_HORIZONTAL),
  dd_label("Destination Folder:\t"),
  dd_fileChooser(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
  display(NULL),
  videoInputSelector(NULL),
  gamepadSelector(NULL)
{
  set_border_width(20);

  vis_refTreeModel = Gtk::ListStore::create(vis_modelColumns);
  vis_refTreeModel->append();
  vis_treeView.set_model(vis_refTreeModel);
  vis_treeView.append_column("PID", vis_modelColumns.pid);
  vis_treeView.append_column("Process Name", vis_modelColumns.processName);
  vis_treeView.append_column("Window Title", vis_modelColumns.windowTitle);
  Glib::RefPtr<Gtk::TreeSelection> vis_treeSelection = vis_treeView.get_selection();
  vis_treeSelection->set_select_function(&select_function);
  vis_box.add(vis_treeView);

  vis_button.signal_clicked().connect(sigc::mem_fun(*this,
    &RecordPage::on_change_window_button_clicked));
  vis_button.set_halign(Gtk::ALIGN_START);
  vis_box.add(vis_button);

  vis_box.set_border_width(10);
  vis_frame.add(vis_box);
  add(vis_frame);

  gs_refTreeModel = Gtk::ListStore::create(gs_modelColumns);
  gs_refTreeModel->append();
  gs_treeView.set_model(gs_refTreeModel);
  gs_treeView.append_column("Device File", gs_modelColumns.deviceFileName);
  gs_treeView.append_column("Device Name", gs_modelColumns.deviceName);
  Glib::RefPtr<Gtk::TreeSelection> gs_treeSelection = gs_treeView.get_selection();
  gs_treeSelection->set_select_function(&select_function);
  gs_box.add(gs_treeView);

  gs_button.signal_clicked().connect(sigc::mem_fun(*this,
    &RecordPage::on_change_gamepad_button_clicked));
  gs_button.set_halign(Gtk::ALIGN_START);
  gs_box.add(gs_button);

  gs_box.set_border_width(10);
  gs_frame.add(gs_box);
  add(gs_frame);

  dd_box.add(dd_label);
  dd_box.add(dd_fileChooser);
  add(dd_box);

  startButton.set_halign(Gtk::ALIGN_END);
  startButton.set_valign(Gtk::ALIGN_END);
  startButton.signal_clicked().connect(sigc::mem_fun(*this,
    &RecordPage::on_startButton_clicked));
  pack_end(startButton);

  show_all_children();
}
