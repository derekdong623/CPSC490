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
  // NB: typeMod contribution; immune is -2.
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
struct SpeciesDict{
  std::unordered_map<Species, Stats> base_stat_dict;
  std::unordered_map<Species, std::pair<Type, Type>> type_dict;
  SpeciesDict();
};
extern SpeciesDict species_dict;

// MOVE DICT
enum class MoveKind {
  NONE,
  STATUS,
  SPECIAL,
  PHYSICAL,
};
struct Move {
  MoveId id;
  Type type;
  MoveKind kind;
  std::optional<int> power;
  std::optional<int> accuracy; // I think no-accuracy is to-ally/self infinite accuracy?
  std::optional<int> init_pp;  // Useful for e.g. Leppa
  std::optional<int> pp;       // no Ivy Cudgel or Max moves special cases; just Struggle
  std::optional<int> prob;
  // Special-case fields (Fury Cutter # times used, mid-flight, turns disabled etc.)
};
struct MoveDict {
  std::unordered_map<MoveId, Move> dict;
  MoveDict();
};
extern MoveDict move_dict;
} // namespace pkmn