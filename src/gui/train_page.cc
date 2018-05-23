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
	bool saveLoss = saveLossCheckButton.get_active();
  bool suspendOnCompletion = suspendCheckButton.get_active();
	uint64_t downsampleRate = downsampleSpinButton.get_value_as_int();
	uint64_t lookback = lookbackSpinButton.get_value_as_int();
	uint64_t totalEpochs = epochSpinButton.get_value_as_int();
  std::string lossDir = lossDirectoryChooser.get_filename();
  std::string lossFileName = lossFileNameEntry.get_text();
  std::string metaFile = modelMetaFileChooser.get_filename();
  std::string trainDataDir = trainDirectoryChooser.get_filename();
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
  else if (saveLoss)
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
      downsampleRate, lookback, totalEpochs, metaFile, trainDataDir);
    trainWindow->signal_delete_event().connect(sigc::mem_fun(*this,
      &train_page::on_train_window_closed));
    trainWindow->set_transient_for(mainWindow);
    trainWindow->show();
  }
}

ssbml::gui::train_page::train_page(Gtk::Window &mainWindow) :
  Box(Gtk::ORIENTATION_VERTICAL, 10),
  mainWindow(mainWindow),

  modelMetaLabel("Meta File:\t"),
	modelMetaBox(Gtk::ORIENTATION_HORIZONTAL, 10),
  createModelButton("Create Model"),
	modelBox(Gtk::ORIENTATION_VERTICAL, 10),
  modelFrame("Model To Train"),

  trainDirectoryLabel("Training Data Directory:\t"),
  trainDirectoryChooser(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
  trainBox(Gtk::ORIENTATION_HORIZONTAL, 10),
  testDirectoryLabel("Testing Data Directory:\t"),
  testDirectoryChooser(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
	testBox(Gtk::ORIENTATION_HORIZONTAL, 10),
	dataBox(Gtk::ORIENTATION_VERTICAL, 10),
	dataFrame("Data Directories"),

	saveLossLabel("Save Loss Values:\t"),
	lossSubBox1(Gtk::ORIENTATION_HORIZONTAL, 10),
	lossDirectoryLabel("Directory:\t"),
	lossDirectoryChooser(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
	lossSubBox2(Gtk::ORIENTATION_HORIZONTAL, 10),
	lossFileNameLabel("File Name:\t"),
	lossSubBox3(Gtk::ORIENTATION_HORIZONTAL, 10),
	lossBox(Gtk::ORIENTATION_VERTICAL, 10),
	lossFrame("Loss Values"),

	epochLabel("Number Of Epochs:\t"),
	epochSpinButton(Gtk::Adjustment::create(10.0, 1.0, 9999.0, 1.0, 10.0, 0.0),
    1, 0),
	epochBox(Gtk::ORIENTATION_HORIZONTAL, 10),

	lookbackLabel("Loopback (seconds):\t"),
	lookbackSpinButton(Gtk::Adjustment::create(3.0, 1.0, 9999.0, 1.0, 10.0, 0.0),
		1, 0),
	lookbackBox(Gtk::ORIENTATION_HORIZONTAL, 10),

	downsampleLabel("Downsample Rate:\t"),
	downsampleSpinButton(Gtk::Adjustment::create(4.0, 1.0, 9999.0, 1.0, 10.0, 0.0),
		1, 0),
	downsampleBox(Gtk::ORIENTATION_HORIZONTAL, 10),

	suspendCheckButton("Suspend OS On Completion:\t"),
	suspendBox(Gtk::ORIENTATION_HORIZONTAL, 10),

  trainButton("Train")
{
  set_border_width(20);

  modelMetaBox.add(modelMetaLabel);
  modelMetaBox.add(modelMetaFileChooser);
  modelBox.add(modelMetaBox);
	createModelButton.signal_clicked().connect(sigc::mem_fun(*this,
    &train_page::on_create_model_button_clicked));
  createModelButton.set_halign(Gtk::ALIGN_START);
  createModelButton.set_valign(Gtk::ALIGN_START);
  modelBox.add(createModelButton);
  modelBox.set_border_width(10);
  modelFrame.add(modelBox);
  add(modelFrame);

  trainBox.add(trainDirectoryLabel);
  trainBox.add(trainDirectoryChooser);
  dataBox.add(trainBox);
  testBox.add(testDirectoryLabel);
  testBox.add(testDirectoryChooser);
  dataBox.add(testBox);
  dataBox.set_border_width(10);
  dataFrame.add(dataBox);
  add(dataFrame);

	lossSubBox1.add(saveLossLabel);
	saveLossCheckButton.signal_toggled().connect(sigc::mem_fun(*this,
		&train_page::on_loss_button_toggled));
	lossSubBox1.add(saveLossCheckButton);
	lossBox.add(lossSubBox1);
	lossSubBox2.add(lossDirectoryLabel);
	lossSubBox2.add(lossDirectoryChooser);
	lossBox.add(lossSubBox2);
	lossSubBox3.add(lossFileNameLabel);
	lossFileNameEntry.set_text("loss.csv");
	lossSubBox3.add(lossFileNameEntry);
	lossBox.add(lossSubBox3);
	lossBox.set_border_width(10);
	lossFrame.add(lossBox);
	add(lossFrame);

  epochBox.add(epochLabel);
  epochBox.add(epochSpinButton);
  add(epochBox);

  lookbackBox.add(lookbackLabel);
  lookbackBox.add(lookbackSpinButton);
  add(lookbackBox);

	downsampleBox.add(downsampleLabel);
	downsampleBox.add(downsampleSpinButton);
	add(downsampleBox);

  suspendBox.add(suspendLabel);
  suspendBox.add(suspendCheckButton);
  add(suspendBox);

  trainButton.signal_clicked().connect(sigc::mem_fun(*this,
    &train_page::on_train_button_clicked));
  trainButton.set_halign(Gtk::ALIGN_END);
  trainButton.set_valign(Gtk::ALIGN_END);
  pack_end(trainButton);

  show_all_children();
}

void ssbml::gui::train_page::on_loss_button_toggled()
{
  bool toggle = saveLossCheckButton.get_active();
  if (toggle)
  {
    lossSubBox2.set_sensitive(true);
    lossSubBox3.set_sensitive(true);
  }
  else
  {
    lossSubBox2.set_sensitive(false);
    lossSubBox3.set_sensitive(false);
  }
}
