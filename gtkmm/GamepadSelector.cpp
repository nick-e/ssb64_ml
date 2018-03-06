#include "GamepadSelector.h"

std::string SSBML::GamepadSelector::get_selected_gamepad()
{

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = treeView.get_selection();
  Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
  if (iter)
  {
    Gtk::TreeModel::Row row = *iter;
    return gamepadDeviceFileNames[row[modelColumns.index]];
  }
  return "";
}

void SSBML::GamepadSelector::refresh()
{
  refTreeModel->clear();
  numGamepads = Gamepad::get_all_connected_gamepad_device_file_names(gamepadDeviceFileNames);

  for (unsigned long i = 0; i < numGamepads; i++)
  {
    std::string deviceFileName = gamepadDeviceFileNames[i];
    Gtk::TreeModel::Row row = *(refTreeModel->append());
    row[modelColumns.deviceFileName] = deviceFileName;
    row[modelColumns.deviceName] = Gamepad::get_device_name(deviceFileName);
  }
}

SSBML::GamepadSelector::ModelColumns::ModelColumns()
{
  add(deviceFileName);
  add(deviceName);
  add(index);
}

bool SSBML::GamepadSelector::on_key_press_event(GdkEventKey *event)
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

SSBML::GamepadSelector::GamepadSelector() : selectButton("Select"),
  box(Gtk::ORIENTATION_VERTICAL, 10), refreshButton("refresh")
{
  set_title("Select A Gamepad");
  set_border_width(0);
  set_default_size(600, 300);
  set_modal(true);

  refTreeModel = Gtk::ListStore::create(modelColumns);
  treeView.set_model(refTreeModel);

  refresh();

  treeView.append_column("Device File", modelColumns.deviceFileName);
  treeView.append_column("Name", modelColumns.deviceName);

  scrolledWindow.add(treeView);
  scrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  box.pack_start(scrolledWindow);

  refreshButton.signal_clicked().connect(sigc::mem_fun(*this,
    &GamepadSelector::refresh));
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
    &GamepadSelector::on_key_press_event));

  show_all_children();
}
