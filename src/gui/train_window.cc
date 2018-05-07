#include "train_window.h"

ssbml::gui::train_window::train_window(bool suspendOnCompletion,
  uint64_t batchSize, uint64_t totalEpochs, std::string metaFile,
  std::string trainingDataDir) :
  epochLabel(""),
  totalBox(Gtk::ORIENTATION_VERTICAL, 10),
  fileNameLabel(""),
  fileLabel(""),
  epochBox(Gtk::ORIENTATION_VERTICAL, 10),
  frameLabel(""),
  fileBox(Gtk::ORIENTATION_VERTICAL, 10),
  ttLabel("Elapsed Time:\t"),
  ttBox(Gtk::ORIENTATION_HORIZONTAL, 10),
  etaLabel("ETA:\t"),
  etaBox(Gtk::ORIENTATION_HORIZONTAL, 10),
  modelLabel("Loading model..."),
  modelBox(Gtk::ORIENTATION_VERTICAL, 10),
  infoBox(Gtk::ORIENTATION_VERTICAL, 10),
  box(Gtk::ORIENTATION_VERTICAL, 10),
  totalEpochs(totalEpochs),
  batchSize(batchSize),
  trainSession(metaFile, trainingDataDir, totalEpochs, batchSize, 256, 144,
    suspendOnCompletion, dispatcher)
{
  set_title("Train Session");
  set_border_width(20);
  set_default_size(600, 300);
  set_modal(true);

  modelLabel.set_halign(Gtk::ALIGN_CENTER);
  modelBox.add(modelLabel);
  modelProgressBar.set_pulse_step(0.1);
  modelBox.add(modelProgressBar);
  box.add(modelBox);

  epochLabel.set_halign(Gtk::ALIGN_START);
  totalBox.add(epochLabel);
  totalProgressBar.set_valign(Gtk::ALIGN_START);
  totalBox.add(totalProgressBar);
  infoBox.add(totalBox);

  fileNameLabel.set_halign(Gtk::ALIGN_START);
  epochBox.add(fileNameLabel);
  fileLabel.set_halign(Gtk::ALIGN_START);
  epochBox.add(fileLabel);
  epochProgressBar.set_valign(Gtk::ALIGN_START);
  epochBox.add(epochProgressBar);
  infoBox.add(epochBox);

  frameLabel.set_halign(Gtk::ALIGN_START);
  fileBox.add(frameLabel);
  fileProgressBar.set_valign(Gtk::ALIGN_START);
  fileBox.add(fileProgressBar);
  infoBox.add(fileBox);

  ttLabel.set_halign(Gtk::ALIGN_START);
  ttBox.add(ttLabel);
  ttBox.add(ttLabel2);
  infoBox.add(ttBox);

  etaLabel.set_halign(Gtk::ALIGN_START);
  etaBox.add(etaLabel);
  etaBox.add(etaLabel2);
  infoBox.add(etaBox);

  infoBox.set_visible(false);
  box.add(infoBox);

  add(box);

  dispatcher.connect(sigc::mem_fun(*this, &train_window::get_train_info));

  show_all_children();
}

void ssbml::gui::train_window::get_train_info()
{
  struct train_session::info info;
  trainSession.get_info(info);

  if (info.modelLoaded)
  {
    totalProgressBar.set_fraction(info.totalProgress);
    epochProgressBar.set_fraction(info.epochProgress);
    fileProgressBar.set_fraction(info.fileProgress);
    if (info.trainingCompleted)
    {
      modelBox.set_visible(true);
      infoBox.set_visible(false);
      modelProgressBar.set_fraction(1.0);
      modelLabel.set_text("Training completed");
    }
    else if (info.totalProgress == 1.0)
    {
      modelBox.set_visible(true);
      infoBox.set_visible(false);
      modelLabel.set_text("Saving model...");
      modelProgressBar.pulse();
    }
    else
    {
      modelBox.set_visible(false);
      infoBox.set_visible(true);

      fileNameLabel.set_text("Current File:\t" + info.currentFileName);
      epochLabel.set_text("Epoch:\t\t\t" + std::to_string(info.currentEpoch)
        + "\t/\t" + std::to_string(totalEpochs));
      fileLabel.set_text("File:\t\t\t" + std::to_string(info.currentFileIndex)
        + "\t/\t" + std::to_string(info.totalFiles));
      frameLabel.set_text("Frames:\t\t" + std::to_string(info.currentFrame)
        + "\t/\t" + std::to_string(info.currentFileFrameCount));
      ttLabel2.set_text(time_to_string(info.timeTaken));
      etaLabel2.set_text(time_to_string(info.eta));
    }
  }
  else
  {
    modelBox.set_visible(true);
    infoBox.set_visible(false);
    modelProgressBar.pulse();
  }
}
