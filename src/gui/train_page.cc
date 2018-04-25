#include "train_page.h"

bool ssbml::gui::train_page::on_train_window_closed(GdkEventAny *event)
{
  delete trainWindow;
  return false;
}

void ssbml::gui::train_page::on_create_model_button_clicked()
{
  Gtk::FileChooserDialog dialog = Gtk::FileChooserDialog(mainWindow,
    "Create File To Store Model", Gtk::FILE_CHOOSER_ACTION_SAVE);
  dialog.set_current_name("model.ckpt");
  dialog.set_transient_for(mainWindow);
  dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
  dialog.add_button("Save", Gtk::RESPONSE_YES);
  dialog.select_filename("model.ckpt");
  int result = dialog.run();
  if (result == Gtk::RESPONSE_YES)
  {
    train_session::create_model(dialog.get_filename());
  }
}

void ssbml::gui::train_page::on_train_button_clicked()
{
  std::string trainDataDir = train_fileChooser.get_filename();
  std::string modelMeta = model_fileChooser.get_filename();
  if (trainDataDir.empty())
  {
    Gtk::MessageDialog dialog(mainWindow, "Missing Training Data Directory",
      false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    dialog.set_secondary_text("You must select a directory containing " \
      "training data");
    dialog.run();
  }
  else if (modelMeta.empty())
  {
    Gtk::MessageDialog dialog(mainWindow, "Missing Model To Train",
      false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    dialog.set_secondary_text("You must select a .meta file");
    dialog.run();
  }
  else
  {
    trainWindow = new ssbml::gui::train_window(modelMeta, trainDataDir,
      epoch_spinButton.get_value_as_int(), batch_spinButton.get_value_as_int());
    trainWindow->signal_delete_event().connect(sigc::mem_fun(*this,
      &train_page::on_train_window_closed));
    trainWindow->set_transient_for(mainWindow);
    trainWindow->show();
  }
}

ssbml::gui::train_page::train_page(Gtk::Window &mainWindow) :
  Box(Gtk::ORIENTATION_VERTICAL, 10),
  mainWindow(mainWindow),
  model_box(Gtk::ORIENTATION_HORIZONTAL, 10),
  model_label1("Model To Train:\t"),
  model_label2("\tOr\t"),
  model_button("Create Model"),
  epoch_box(Gtk::ORIENTATION_HORIZONTAL, 10),
  epoch_label("Number Of Epochs:\t"),
  epoch_spinButton(Gtk::Adjustment::create(20.0, 1.0, 9999.0, 1.0, 10.0, 0.0),
    1, 0),
  batch_box(Gtk::ORIENTATION_HORIZONTAL, 10),
  batch_label("Batch Size:\t\t\t"),
  batch_spinButton(Gtk::Adjustment::create(10.0, 1.0, 9999.0, 1.0, 10.0, 0.0),
    1, 0),
  data_frame("Data Directories"),
  data_box(Gtk::ORIENTATION_VERTICAL, 10),
  train_box(Gtk::ORIENTATION_HORIZONTAL, 10),
  train_label("Training Data Directory:\t"),
  train_fileChooser(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
  test_box(Gtk::ORIENTATION_HORIZONTAL, 10),
  test_label("Testing Data Directory:\t"),
  test_fileChooser(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
  trainButton("Train")
{
  set_border_width(20);

  model_button.signal_clicked().connect(sigc::mem_fun(*this,
    &train_page::on_create_model_button_clicked));

  model_box.add(model_label1);
  model_box.add(model_fileChooser);
  model_box.add(model_label2);
  model_box.add(model_button);
  add(model_box);

  train_box.add(train_label);
  train_box.add(train_fileChooser);
  data_box.add(train_box);

  test_box.add(test_label);
  test_box.add(test_fileChooser);
  data_box.add(test_box);

  data_box.set_border_width(10);
  data_frame.add(data_box);
  add(data_frame);

  epoch_box.add(epoch_label);
  epoch_box.add(epoch_spinButton);
  add(epoch_box);

  batch_box.add(batch_label);
  batch_box.add(batch_spinButton);
  add(batch_box);

  trainButton.signal_clicked().connect(sigc::mem_fun(*this,
    &train_page::on_train_button_clicked));
  trainButton.set_halign(Gtk::ALIGN_END);
  trainButton.set_valign(Gtk::ALIGN_END);
  pack_end(trainButton);

  show_all_children();
}
