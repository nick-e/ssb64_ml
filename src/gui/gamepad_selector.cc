#include "gamepad_selector.h"

static bool select_function(const Glib::RefPtr<Gtk::TreeModel>& model,
  const Gtk::TreeModel::Path& path, bool pathCurrentlySelected)
{
  return false;
}

void ssbml::gui::gamepad_selector::on_button_clicked()
{
  gamepadSelectorPopup = new ssbml::gui::gamepad_selector_popup();
  gamepadSelectorPopup->selectButton.signal_clicked().connect(sigc::mem_fun(
    *this, &gamepad_selector::on_gamepad_selector_popup_select_button_clicked));
  gamepadSelectorPopup->treeView.signal_row_activated().connect(sigc::mem_fun(
    *this, &gamepad_selector::on_gamepad_selector_popup_row_activated));
  gamepadSelectorPopup->set_transient_for(mainWindow);
  gamepadSelectorPopup->show();
}

void ssbml::gui::gamepad_selector::on_gamepad_selector_popup_row_activated(
  const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column)
{
  get_selected_gamepad();
}

void ssbml::gui::gamepad_selector::on_gamepad_selector_popup_select_button_clicked()
{
  get_selected_gamepad();
}

void ssbml::gui::gamepad_selector::get_selected_gamepad()
{
  selectedGamepadDeviceFileName = gamepadSelectorPopup->get_selected_gamepad();
  refTreeModel->clear();
  Gtk::TreeModel::Row row = *(refTreeModel->append());
  row[modelColumns.deviceFileName] = selectedGamepadDeviceFileName;
  row[modelColumns.deviceName] = gamepad_listener::get_device_name(
    selectedGamepadDeviceFileName);
  gamepadSelectorPopup->close();
}

std::string ssbml::gui::gamepad_selector::get_selected_gamepad_device_file_name()
{
  return selectedGamepadDeviceFileName;
}

ssbml::gui::gamepad_selector::gamepad_selector(std::string title,
  Gtk::Window &mainWindow) :
  Gtk::Frame(title),
  mainWindow(mainWindow),
  box(Gtk::ORIENTATION_VERTICAL, 10),
  button("Change"),
  gamepadSelectorPopup(nullptr),
  selectedGamepadDeviceFileName("")
{
  refTreeModel = Gtk::ListStore::create(modelColumns);
  refTreeModel->append();
  treeView.set_model(refTreeModel);
  treeView.append_column("Device File", modelColumns.deviceFileName);
  treeView.append_column("Device Name", modelColumns.deviceName);
  Glib::RefPtr<Gtk::TreeSelection> treeSelection = treeView.get_selection();
  treeSelection->set_select_function(&select_function);
  box.add(treeView);

  button.signal_clicked().connect(sigc::mem_fun(*this,
    &gamepad_selector::on_button_clicked));
  button.set_halign(Gtk::ALIGN_START);
  box.add(button);

  box.set_border_width(10);
  add(box);
}
