#include "ref.hpp"

#include <format>
#include <optional>
namespace pkmn {
// GLOBAL VARIABLE
std::string DATA_FOLDER = "base_gen_8";
// std::string DATA_FOLDER = "run_and_bun";

// TYPE CHART
TypeChart type_chart;
TypeChart::TypeChart() {
  const std::unordered_set<std::pair<Type, Type>> super_eff = {
      {Type::FIRE, Type::GRASS},      {Type::FIRE, Type::ICE},
      {Type::FIRE, Type::BUG},        {Type::FIRE, Type::STEEL},
      {Type::WATER, Type::FIRE},      {Type::WATER, Type::GROUND},
      {Type::WATER, Type::ROCK},      {Type::ELECTRIC, Type::WATER},
      {Type::ELECTRIC, Type::FLYING}, {Type::GRASS, Type::WATER},
      {Type::GRASS, Type::GROUND},    {Type::GRASS, Type::ROCK},
      {Type::ICE, Type::GRASS},       {Type::ICE, Type::GROUND},
      {Type::ICE, Type::FLYING},      {Type::ICE, Type::DRAGON},
      {Type::FIGHTING, Type::NORMAL}, {Type::FIGHTING, Type::ICE},
      {Type::FIGHTING, Type::ROCK},   {Type::FIGHTING, Type::DARK},
      {Type::FIGHTING, Type::STEEL},  {Type::POISON, Type::GRASS},
      {Type::POISON, Type::FAIRY},    {Type::GROUND, Type::FIRE},
      {Type::GROUND, Type::ELECTRIC}, {Type::GROUND, Type::POISON},
      {Type::GROUND, Type::ROCK},     {Type::GROUND, Type::STEEL},
      {Type::FLYING, Type::GRASS},    {Type::FLYING, Type::FIGHTING},
      {Type::FLYING, Type::BUG},      {Type::PSYCHIC, Type::FIGHTING},
      {Type::PSYCHIC, Type::POISON},  {Type::BUG, Type::GRASS},
      {Type::BUG, Type::PSYCHIC},     {Type::BUG, Type::DARK},
      {Type::ROCK, Type::FIRE},       {Type::ROCK, Type::ICE},
      {Type::ROCK, Type::FLYING},     {Type::ROCK, Type::BUG},
      {Type::GHOST, Type::PSYCHIC},   {Type::GHOST, Type::GHOST},
      {Type::DRAGON, Type::DRAGON},   {Type::DARK, Type::PSYCHIC},
      {Type::DARK, Type::GHOST},      {Type::STEEL, Type::ICE},
      {Type::STEEL, Type::ROCK},      {Type::STEEL, Type::FAIRY},
      {Type::FAIRY, Type::FIGHTING},  {Type::FAIRY, Type::DRAGON},
      {Type::FAIRY, Type::DARK}};
  const std::unordered_set<std::pair<Type, Type>> not_eff = {
      {Type::NORMAL, Type::ROCK},     {Type::NORMAL, Type::STEEL},
      {Type::FIRE, Type::FIRE},       {Type::FIRE, Type::WATER},
      {Type::FIRE, Type::ROCK},       {Type::FIRE, Type::DRAGON},
      {Type::WATER, Type::WATER},     {Type::WATER, Type::GRASS},
      {Type::WATER, Type::DRAGON},    {Type::ELECTRIC, Type::ELECTRIC},
      {Type::ELECTRIC, Type::GRASS},  {Type::ELECTRIC, Type::DRAGON},
      {Type::GRASS, Type::FIRE},      {Type::GRASS, Type::GRASS},
      {Type::GRASS, Type::POISON},    {Type::GRASS, Type::FLYING},
      {Type::GRASS, Type::BUG},       {Type::GRASS, Type::DRAGON},
      {Type::GRASS, Type::STEEL},     {Type::ICE, Type::FIRE},
      {Type::ICE, Type::WATER},       {Type::ICE, Type::ICE},
      {Type::ICE, Type::STEEL},       {Type::FIGHTING, Type::POISON},
      {Type::FIGHTING, Type::FLYING}, {Type::FIGHTING, Type::PSYCHIC},
      {Type::FIGHTING, Type::BUG},    {Type::FIGHTING, Type::FAIRY},
      {Type::POISON, Type::POISON},   {Type::POISON, Type::GROUND},
      {Type::POISON, Type::ROCK},     {Type::POISON, Type::GHOST},
      {Type::GROUND, Type::GRASS},    {Type::GROUND, Type::BUG},
      {Type::FLYING, Type::ELECTRIC}, {Type::FLYING, Type::ROCK},
      {Type::FLYING, Type::STEEL},    {Type::PSYCHIC, Type::PSYCHIC},
      {Type::PSYCHIC, Type::STEEL},   {Type::BUG, Type::FIRE},
      {Type::BUG, Type::FIGHTING},    {Type::BUG, Type::POISON},
      {Type::BUG, Type::FLYING},      {Type::BUG, Type::GHOST},
      {Type::BUG, Type::STEEL},       {Type::BUG, Type::FAIRY},
      {Type::ROCK, Type::FIGHTING},   {Type::ROCK, Type::GROUND},
      {Type::ROCK, Type::STEEL},      {Type::GHOST, Type::DARK},
      {Type::DRAGON, Type::STEEL},    {Type::DARK, Type::FIGHTING},
      {Type::DARK, Type::DARK},       {Type::DARK, Type::FAIRY},
      {Type::STEEL, Type::FIRE},      {Type::STEEL, Type::WATER},
      {Type::STEEL, Type::ELECTRIC},  {Type::STEEL, Type::STEEL},
      {Type::FAIRY, Type::FIRE},      {Type::FAIRY, Type::POISON},
      {Type::FAIRY, Type::STEEL}};
  const std::unordered_set<std::pair<Type, Type>> immune = {
      {Type::NORMAL, Type::GHOST}, {Type::ELECTRIC, Type::GROUND}, {Type::FIGHTING, Type::GHOST},
      {Type::POISON, Type::STEEL}, {Type::GROUND, Type::FLYING},   {Type::PSYCHIC, Type::DARK},
      {Type::GHOST, Type::NORMAL}, {Type::DRAGON, Type::FAIRY}};
  for (int i = 1; i < (int)Type::END; i++) { // We know that there are 18 types; 0 is NONE
    Type attacker = (Type)i;
    for (int j = 1; j < (int)Type::END; j++) {
      Type defender = (Type)j;
      std::pair<Type, Type> type_pair = {attacker, defender};
      if (super_eff.find(type_pair) != super_eff.end()) {
        chart[attacker][defender] = 1; // Super effective
      } else if (not_eff.find(type_pair) != not_eff.end()) {
        chart[attacker][defender] = -1; // Not very effective
      } else if (immune.find(type_pair) != immune.end()) {
        chart[attacker][defender] = -2; // Immune
      } else {
        chart[attacker][defender] = 0; // Neutral
      }
    }
  }
}
int get_type_effectiveness(Type attacker, Type defender) {
  return type_chart.chart[attacker][defender];
}

// BASE STATS and TYPES
SpeciesDict species_dict;
SpeciesDict::SpeciesDict() {
  std::ifstream file(std::format("../data/{}/species_data_normal.txt", DATA_FOLDER));
  if (!file) {
    std::cerr << "Error opening datafile of normal species!" << std::endl;
  }
  int i = 0;
  std::string line;
  while (std::getline(file, line)) {
    if (++i >= (int)Species::END) {
      std::cerr << "Extra line while reading stats' datafile:" << std::endl;
      std::cerr << line << std::endl;
      break;
    }
    std::istringstream iss(line);
    Stats stats{};
    int a, b;
    iss >> stats.hp >> stats.att >> stats.def >> stats.spatt >> stats.spdef >> stats.spd >> a >> b;
    Species species = (Species)i;
    base_stat_dict[species] = stats;
    type_dict[species] = std::pair<Type, Type>{static_cast<Type>(a), static_cast<Type>(b)};
  }
  file.close();
}

// MOVES
MoveDict move_dict;
std::optional<int> get_opt_nonneg_int(int val) {
  return val >= 0 ? val : std::optional<int>(std::nullopt);
}
MoveDict::MoveDict() {
  // Read from file
  std::ifstream file(std::format("../data/{}/moves/move_data.txt", DATA_FOLDER));
  if (!file) {
    std::cerr << "Error opening moves' datafile!" << std::endl;
  }
  std::string line;
  for (int j = 1; j < (int)MoveId::END; j++) {
    MoveId id = (MoveId)j;
    int tmp[6];
    if(!std::getline(file, line)) {
      std::cerr << "Not enough lines while reading moves' datafile:" << std::endl;
      break;
    }
    std::istringstream iss(line);
    Move ret{};
    for(int k=0; k<6; k++) {
      iss >> tmp[k];
    }
    ret.type = static_cast<Type>(tmp[0]);
    ret.kind = static_cast<MoveKind>(tmp[1]);
    ret.power = get_opt_nonneg_int(tmp[2]);
    ret.accuracy = get_opt_nonneg_int(tmp[3]);
    ret.pp = get_opt_nonneg_int(tmp[4]);
    ret.prob = get_opt_nonneg_int(tmp[5]);
    ret.init_pp = ret.pp;
    dict[id] = ret;
  }
  if(std::getline(file, line)) {
    std::cerr << "Extra line(s) while reading moves' datafile:" << std::endl;
    std::cerr << line << std::endl;
  }
  file.close();
}
} // namespace pkmn