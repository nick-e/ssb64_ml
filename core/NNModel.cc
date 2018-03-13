#include "NNModel.h"

void SSBML::NNModel::create_model(std::string dst)
{
  char buf[1024];
  FILE *in = popen(("python ../core/createModel.py " + dst + " 256 144 3 14 14").c_str(), "r");
  size_t length;
  while ((length = fread(buf, 1, sizeof(buf), in)))
  {
    if (ferror(in))
    {
      perror("SSBML::NNModel::create_model(): fread()");
      break;
    }
    if (length > 0)
    {
      std::cout << buf;
    }
    if (feof(in))
    {
      break;
    }
  }
  pclose(in);
}

void SSBML::NNModel::train_model(std::string src, std::string trainingDataDir)
{
  /*char buf[1024];
  FILE *in = popen(("python ../core/trainModel.py " + src + " 10 10 256 144 3 14 " + trainingDataDir).c_str(), "r");
  size_t length;
  while ((length = fread(buf, 1, sizeof(buf), in)))
  {
    if (ferror(in))
    {
      perror("SSBML::NNModel::create_model(): fread()");
      break;
    }
    if (length > 0)
    {
      std::cout << buf;
    }
    if (feof(in))
    {
      break;
    }
  }
  pclose(in);*/
  system(("python ../core/trainModel.py " + src + " 10 10 256 144 3 14 " + trainingDataDir).c_str());
}
