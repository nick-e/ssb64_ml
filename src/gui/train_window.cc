#include "train_window.h"

ssbml::gui::train_window::train_window(std::string modelDir,
  std::string trainingDataDir, uint64_t epochs, uint64_t batchSize) :
  modelLabel("Loading model..."),
  epochLabel(""),
  fileLabel(""),
  frameLabel(""),
  fileNameLabel(""),
  infoBox(Gtk::ORIENTATION_VERTICAL, 10),
  box(Gtk::ORIENTATION_VERTICAL, 10),
  epochs(epochs),
  batchSize(batchSize),
  trainSession(modelDir, trainingDataDir, epochs, batchSize, 256, 144,
    dispatcher)
{
  set_title("Train Session");
  set_border_width(20);
  set_default_size(600, 300);
  set_modal(true);

  progressBar.set_valign(Gtk::ALIGN_START);
  box.add(progressBar);

  box.add(modelLabel);

  fileNameLabel.set_halign(Gtk::ALIGN_START);
  infoBox.add(fileNameLabel);
  frameLabel.set_halign(Gtk::ALIGN_START);
  infoBox.add(frameLabel);
  fileLabel.set_halign(Gtk::ALIGN_START);
  infoBox.add(fileLabel);
  epochLabel.set_halign(Gtk::ALIGN_START);
  infoBox.add(epochLabel);
  infoBox.set_visible(false);
  box.add(infoBox);

  add(box);

  dispatcher.connect(sigc::mem_fun(*this, &train_window::get_train_info));

  show_all_children();
}

void ssbml::gui::train_window::get_train_info()
{
  bool modelLoaded;
  uint64_t currentEpoch;
  uint64_t currentFile;
  uint64_t currentFrame;
  uint64_t currentFileFrameCount;
  std::string currentFileName;
  double progress;
  uint64_t fileCount;
  bool doneTraining;

  trainSession.get_train_info(&modelLoaded, &currentEpoch, &currentFile,
    &currentFileFrameCount, currentFileName, &currentFrame, &progress,
    &fileCount, &doneTraining);
  if (modelLoaded)
  {
    progressBar.set_fraction(progress);
    if (doneTraining)
    {
      modelLabel.set_visible(true);
      infoBox.set_visible(false);
      modelLabel.set_text("Training completed");
    }
    else if (progress == 1.0)
    {
      modelLabel.set_visible(true);
      infoBox.set_visible(false);
      modelLabel.set_text("Saving model...");
    }
    else
    {
      modelLabel.set_visible(false);
      infoBox.set_visible(true);

      fileNameLabel.set_text("Current File:\t" + currentFileName);
      epochLabel.set_text("Epoch:\t\t\t" + std::to_string(currentEpoch) + " / "
        + std::to_string(epochs));
      fileLabel.set_text("File:\t\t\t" + std::to_string(currentFile) + " / "
        + std::to_string(fileCount));
      frameLabel.set_text("Frames:\t\t" + std::to_string(currentFrame) + " / "
        + std::to_string(currentFileFrameCount));
    }
  }
  else
  {
    modelLabel.set_visible(true);
    infoBox.set_visible(false);
    progressBar.pulse();
  }
}
