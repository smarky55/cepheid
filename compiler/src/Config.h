#pragma once

#include <nlohmann/json_fwd.hpp>
#include <string>

namespace Cepheid {
class Config {
 public:
  [[nodiscard]] std::string_view name() const;
  [[nodiscard]] std::string_view description() const;

  friend void to_json(nlohmann::json& json, const Config& config);
  friend void from_json(const nlohmann::json& json, Config& config);
 private:
  std::string m_name;
  std::string m_description;
};


}  // namespace Cepheid