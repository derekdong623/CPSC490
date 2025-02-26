#include "ref.hpp"

namespace pkmn {
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
    for (int i = 0; i < (int)Type::COUNT; i++) { // We know that there are 18 types; 0 is NONE
      Type attacker = (Type)i;
      for (int j = 0; j <= (int)Type::COUNT; j++) {
        Type defender = (Type)j;
        std::pair<Type, Type> type_pair = {attacker, defender};
        if (super_eff.find(type_pair) != super_eff.end()) {
          chart[attacker][defender] = 4;
        } else if (not_eff.find(type_pair) != not_eff.end()) {
          chart[attacker][defender] = 1;
        } else if (immune.find(type_pair) != immune.end()) {
          chart[attacker][defender] = 0;
        } else {
          chart[attacker][defender] = 2;
        }
      }
    }
  }
int get_type_effectiveness(Type attacker, Type defender) {
return type_chart.chart[attacker][defender];
}

// BASE STATS
BaseStatDict base_stat_dict;
BaseStatDict::BaseStatDict() {
  std::ifstream file("../data/base_gen_8/base_stats_normal.txt");
  if (!file) {
    std::cerr << "Error opening datafile of normal base stats!" << std::endl;
  }
  int i = 0;
  std::string line;
  while (std::getline(file, line)) {
    if (++i >= (int)Species::END) {
      std::cerr << "Extra line:" << std::endl;
      std::cerr << line << std::endl;
      break;
    }
    std::istringstream iss(line);
    Stats ret{};
    iss >> ret.hp >> ret.att >> ret.def >> ret.spatt >> ret.spdef >> ret.spd;
    Species species = (Species)i;
    dict[species] = ret;
  }
  file.close();
}

BaseStatDict form_base_stat_dict;
}