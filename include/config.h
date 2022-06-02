#ifndef __CONFIG_H
#define __CONFIG_H

#include <vector>
#include <string>
#include <unordered_map>

class Config {
public:
  ~Config();
  static Config *getInstance();

  /**
   * @brief load the configuration from the config file
   *
   * @param path
   * @return true = success, false = failed
   */
  bool load(const std::string &path);

  /**
   * @brief Get the configuration value with the provided key
   *
   * @return std::string
   */
  std::string getValue(std::string key);

  void printConfig(void) const;

private:
  Config();

private:
  static Config *m_instance;
  std::unordered_map<std::string, std::string> m_items;
};

#endif