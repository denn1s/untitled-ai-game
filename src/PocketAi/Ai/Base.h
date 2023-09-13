#pragma once

#include <string>
#include <atomic>

class Base {
public:
  virtual void initialize() = 0;
  virtual void process(const std::string& prompt) = 0;
  virtual void sample() = 0;
  virtual void retrain(const std::string& promptFile) = 0;
  std::atomic<bool> isInitialized;

protected:
  std::string username;
  std::string ainame;
};

