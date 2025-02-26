#include "pokemon.hpp"
#include "test_framework.hpp"
#include <iostream>
namespace pkmn {
bool test_init_type_chart() {
  if (get_type_effectiveness(Type::BUG, Type::WATER) != 2)
    return false; // Normal effectiveness
  if (get_type_effectiveness(Type::BUG, Type::FIRE) != 1)
    return false; // Not very effective
  if (get_type_effectiveness(Type::BUG, Type::PSYCHIC) != 4)
    return false; // Super effective
  if (get_type_effectiveness(Type::DRAGON, Type::FAIRY) != 0)
    return false; // Immunity
  return true;
}
bool test_read_base_stats() {
  if (base_stat_dict.dict[Species::BULBASAUR].hp != 45)
    return false;
  return true;
}
bool test_init_piplup() {
  // TODO
  Pokemon piplup = Pokemon(Species::PIPLUP, // Species
                           100,             // Level
                           Nature::HARDY,   // Nature
                           Stats{},         // IVs
                           Stats{}          // EVs
  );
  return true;
}
} // namespace pkmn
std::vector<testing::TestCase> init_tests = {
    {"Initialize type chart", pkmn::test_init_type_chart},
    {"Initialize base stats dict", pkmn::test_read_base_stats},
    {"Initialize example Pokemon, Piplup", pkmn::test_init_piplup}};