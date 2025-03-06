#include "battle_state.hpp"
#include "pokemon.hpp"
#include "test_framework.hpp"
#include <iostream>
namespace pkmn {
bool test_damage_dealt() {
  // Neither Lillipup nor Tackle are changed in Run and Bun
  Pokemon attacker = Pokemon(Species::LILLIPUP, 10, Nature::HARDY, Stats{}, Stats{});
  Pokemon defender = Pokemon(Species::LILLIPUP, 5, Nature::HARDY, Stats{}, Stats{});
  // Check the max rolled damage
  auto max_dmg = get_deterministic_dmg_roll(attacker, defender, move_dict.dict[MoveId::TACKLE], false, 15);
  if(max_dmg != 16) return false; // From Showdown calculator
  return true;
}
bool test_basic_damage() {
  BattleState start_state;
  start_state.set_move_options();
  return true;
}
} // namespace pkmn
std::vector<testing::TestCase> battle_tests = {
    {"Very basic attack damage test", pkmn::test_damage_dealt},
};