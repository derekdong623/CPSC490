#include "pokemon.hpp"
#include "test_framework.hpp"
#include <iostream>
namespace pkmn {
bool test_init_type_chart() {
  if (getTypeEffectiveness(Type::BUG, Type::WATER) != 0)
    return false; // Normal effectiveness
  if (getTypeEffectiveness(Type::BUG, Type::FIRE) != -1)
    return false; // Not very effective
  if (getTypeEffectiveness(Type::BUG, Type::PSYCHIC) != 1)
    return false; // Super effective
  if (getTypeEffectiveness(Type::DRAGON, Type::FAIRY) != -2)
    return false; // Immunity
  return true;
}
bool test_read_base_stats() {
  // Bulbasaur has no changes in Run and Bun
  int bulb_hp = pokedex.base_stat_dict[PokeName::BULBASAUR].hp;
  if (bulb_hp != 45) {
    std::cout << bulb_hp << std::endl;
    return false;
  }
  return true;
}
bool test_init_lillipup() {
  // Lillipup has no changes in Run and Bun
  Pokemon lillipup = Pokemon(PokeName::LILLIPUP, 10, Gender::MALE, Nature::HARDY, Stats{}, Stats{});
  if (lillipup.init_stats.hp != 29)
    return false;
  if (lillipup.init_stats.att != 17)
    return false;
  if (lillipup.init_stats.def != 14)
    return false;
  if (lillipup.init_stats.spatt != 10)
    return false;
  if (lillipup.init_stats.spdef != 14)
    return false;
  if (lillipup.init_stats.spd != 16)
    return false;
  return true;
}
bool test_init_moves() {
  Move tackle = moveDict.dict[MoveId::TACKLE];
  if(tackle.basePower != 40) {
    return false;
  }
  return true;
}
} // namespace pkmn
std::vector<testing::TestCase> init_tests = {
    {"Initialize type chart", pkmn::test_init_type_chart},
    {"Initialize base stats dict", pkmn::test_read_base_stats},
    {"Initialize example Pokemon, Lillipup", pkmn::test_init_lillipup},
    {"Initialize example move(s)", pkmn::test_init_moves}};