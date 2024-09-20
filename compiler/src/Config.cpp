#include "Config.h"

#include <nlohmann/json.hpp>

std::string_view Cepheid::Config::name() const {
  return m_name;
}

std::string_view Cepheid::Config::description() const {
  return m_description;
}

void Cepheid::to_json(nlohmann::json& json, const Config& config) {
  json = nlohmann::json{{"name", config.m_name}, {"description", config.m_description}};
}

#define GET_VALUE(config, json, key, default) config.m_##key = json.value(#key, default)
void Cepheid::from_json(const nlohmann::json& json, Config& config) {
  GET_VALUE(config, json, name, "");
  GET_VALUE(config, json, description, "");
}
