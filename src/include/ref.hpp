#pragma once

#include "pokemon_enums.hpp"
#include <unordered_map>
#include <map>
namespace pkmn {
// TYPE CHART
struct TypeChart {
  // NB: typeMod contribution; immune is -2.
  std::unordered_map<Type, std::unordered_map<Type, int>> chart;
  TypeChart();
};
extern TypeChart type_chart;
int getTypeEffectiveness(Type attacker, Type defender);

// BASE STATS
struct Stats {
  int hp = 0;
  int att = 0;
  int def = 0;
  int spatt = 0;
  int spdef = 0;
  int spd = 0;
};
using ModifierTable = std::map<ModifierId, int>;
struct PokeDexDict {
  std::unordered_map<PokeName, Stats> base_stat_dict;
  std::unordered_map<PokeName, std::pair<Type, Type>> type_dict;
  std::unordered_map<PokeName, Species> species_dict;
  PokeDexDict();
};
extern PokeDexDict pokedex;

// MOVE DICT
enum class MoveCategory {
  NONE,
  STATUS,
  SPECIAL,
  PHYSICAL,
};
struct Move {
  MoveId id;
  Type type;
  int basePower;
  std::optional<int> accuracy; // I think no-accuracy is to-ally/self infinite accuracy?
  int pp;
  int critRatio;
  int priority;
  MoveCategory category;
  Status status;
  Weather weather;
  Terrain terrain;
  Target target;
  std::optional<int> init_pp; // Useful for e.g. Leppa
  // Flags
  bool sound = false;
  // Special-case fields (Fury Cutter # times used, mid-flight, turns disabled etc.)
};
struct MoveDict {
  std::unordered_map<MoveId, Move> dict;
  MoveDict();
  void patch_move_dict();
};
extern MoveDict moveDict;
} // namespace pkmn