#ifndef NNMODEL_H
#define NNMODEL_H

#include <string>
#include <iostream>

namespace SSBML
{
  class NNModel
  {
  public:
    static void create_model(std::string dst);
    static void train_model(std::string src, std::string trainingDataDir);
  };
}

#endif
