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
  bool suspendOnCompletion = loss_button.get_active();
  std::string lossDir = loss_fileChooser.get_filename();
  std::string lossFileName = loss_entry.get_text();
  std::string metaFile = model_fileChooser.get_filename();
  std::string trainDataDir = train_fileChooser.get_filename();
  if (trainDataDir.empty())
  {
    Gtk::MessageDialog dialog(mainWindow, "Missing Training Data Directory",
      false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    dialog.set_secondary_text(
      "You must select a directory containing training data");
    dialog.run();
  }
  else if (metaFile.empty())
  {
    Gtk::MessageDialog dialog(mainWindow, "Missing Model To Train",
      false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    dialog.set_secondary_text("You must select a .meta file");
    dialog.run();
  }
  else if (suspendOnCompletion)
  {
    if (lossDir.empty())
    {
      Gtk::MessageDialog dialog(mainWindow, "Missing Loss Directory",
        false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
      dialog.set_secondary_text(
        "You must select a directory to save the loss file");
      dialog.run();
    }
    else if (lossFileName.empty())
    {
      Gtk::MessageDialog dialog(mainWindow, "Missing Loss File Name",
        false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
      dialog.set_secondary_text(
        "You must enter a file name for where to save the loss values");
      dialog.run();
    }
  }
  else
  {
    trainWindow = new ssbml::gui::train_window(suspendOnCompletion,
      batch_spinButton.get_value_as_int(), epoch_spinButton.get_value_as_int(),
      metaFile, trainDataDir);
    trainWindow->signal_delete_event().connect(sigc::mem_fun(*this,
      &train_page::on_train_window_closed));
    trainWindow->set_transient_for(mainWindow);
    trainWindow->show();
  }
}

ssbml::gui::train_page::train_page(Gtk::Window &mainWindow) :
  Box(Gtk::ORIENTATION_VERTICAL, 10),
  mainWindow(mainWindow),
  model_frame("Model To Train"),
  model_box(Gtk::ORIENTATION_VERTICAL, 10),
  model_box2(Gtk::ORIENTATION_HORIZONTAL, 10),
  model_label("Meta File:\t"),
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
  loss_frame("Loss Values"),
  loss_box(Gtk::ORIENTATION_VERTICAL, 10),
  loss_box2(Gtk::ORIENTATION_HORIZONTAL, 10),
  loss_box3(Gtk::ORIENTATION_HORIZONTAL, 10),
  loss_box4(Gtk::ORIENTATION_HORIZONTAL, 10),
  loss_label("Save Loss Values:\t"),
  loss_fileChooser(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
  loss_label2("Directory:\t\t"),
  loss_label3("File Name:\t"),
  suspend_label("Suspend OS On Completion:\t"),
  suspend_box(Gtk::ORIENTATION_HORIZONTAL, 10),
  trainButton("Train")
{
  set_border_width(20);

  model_button.signal_clicked().connect(sigc::mem_fun(*this,
    &train_page::on_create_model_button_clicked));

  model_box2.add(model_label);
  model_box2.add(model_fileChooser);
  model_box.add(model_box2);

  model_button.set_halign(Gtk::ALIGN_START);
  model_button.set_valign(Gtk::ALIGN_START);
  model_box.add(model_button);

  model_box.set_border_width(10);
  model_frame.add(model_box);
  add(model_frame);

  train_box.add(train_label);
  train_box.add(train_fileChooser);
  data_box.add(train_box);

  test_box.add(test_label);
  test_box.add(test_fileChooser);
  data_box.add(test_box);

  data_box.set_border_width(10);
  data_frame.add(data_box);
  add(data_frame);

  loss_box2.add(loss_label);
  loss_button.set_active(true);
  loss_button.signal_toggled().connect(sigc::mem_fun(*this,
    &train_page::on_loss_button_toggled));
  loss_box2.add(loss_button);
  loss_box.add(loss_box2);

  loss_box3.add(loss_label2);
  loss_box3.add(loss_fileChooser);
  loss_box.add(loss_box3);

  loss_box4.add(loss_label3);
  loss_entry.set_text("loss.csv");
  loss_box4.add(loss_entry);
  loss_box.add(loss_box4);

  loss_box.set_border_width(10);
  loss_frame.add(loss_box);
  add(loss_frame);

  epoch_box.add(epoch_label);
  epoch_box.add(epoch_spinButton);
  add(epoch_box);

  batch_box.add(batch_label);
  batch_box.add(batch_spinButton);
  add(batch_box);

  suspend_box.add(suspend_label);
  suspend_box.add(suspend_button);
  add(suspend_box);

  trainButton.signal_clicked().connect(sigc::mem_fun(*this,
    &train_page::on_train_button_clicked));
  trainButton.set_halign(Gtk::ALIGN_END);
  trainButton.set_valign(Gtk::ALIGN_END);
  pack_end(trainButton);

  show_all_children();
}

void ssbml::gui::train_page::on_loss_button_toggled()
{
  bool toggle = loss_button.get_active();
  if (toggle)
  {
    loss_box3.set_sensitive(true);
    loss_box4.set_sensitive(true);
  }
  else
  {
    loss_box3.set_sensitive(false);
    loss_box4.set_sensitive(false);
  }
}
