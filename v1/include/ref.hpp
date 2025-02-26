#pragma once

#include "pokemon_enums.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
namespace pkmn {
// TYPE CHART
struct TypeChart {
  // NB: Maps to 2 * effectiveness, i.e. 0.5 is represented by 1.
  std::unordered_map<Type, std::unordered_map<Type, int>> chart;
  TypeChart();
};
extern TypeChart type_chart;
int get_type_effectiveness(Type attacker, Type defender);

// BASE STATS
struct Stats {
  int hp{}, att{}, def{}, spatt{}, spdef{}, spd{};
};
struct ModifierTable {
  int hp{}, att{}, def{}, spatt{}, spdef{}, spd{}, acc{}, eva{};
};
struct BaseStatDict {
  std::unordered_map<Species, Stats> dict;
  BaseStatDict();
};
extern BaseStatDict base_stat_dict;
} // namespace pkmn