#include "battle_state.hpp"
#include "test_framework.hpp"
#include <iostream>
#include <chrono>


#define TIME_EXPR(label, code) do { \
  auto start = std::chrono::high_resolution_clock::now(); \
  code \
  auto end = std::chrono::high_resolution_clock::now(); \
  std::chrono::duration<double, std::milli> elapsed = end - start; \
  std::cout << label << ": " << elapsed.count() << " ms\n"; \
} while(0)
#define TIME_EXPR_RET(label, expr) [&]() { \
  auto start = std::chrono::high_resolution_clock::now(); \
  auto result = (expr); \
  auto end = std::chrono::high_resolution_clock::now(); \
  std::chrono::duration<double, std::milli> elapsed = end - start; \
  std::cout << label << ": " << elapsed.count() << " ms\n"; \
  return result; \
}()
namespace pkmn {
bool test_initialize_battle() {
  Pokemon charmander =
      Pokemon(PokeName::CHARMANDER, 5, Gender::MALE, Nature::HARDY, Stats{}, Stats{});
  Pokemon bulbasaur =
      Pokemon(PokeName::BULBASAUR, 5, Gender::MALE, Nature::HARDY, Stats{}, Stats{});
  charmander.set_ability(Ability::BLAZE);
  bulbasaur.set_ability(Ability::OVERGROW);
  // There shouldn't be any overwriting from this setup
  if (charmander.add_move(MoveId::SCRATCH, 0)) {
    return false;
  }
  if (charmander.add_move(MoveId::EMBER, 1)) {
    return false;
  }
  if (bulbasaur.add_move(MoveId::TACKLE, 0)) {
    return false;
  }
  Team team0, team1;
  team0.add_pokemon(0, charmander);
  team0.add_pokemon(1, bulbasaur);
  team1.add_pokemon(0, bulbasaur);
  BattleState state;
  if (state.teams[0].activeInd != -1)
    return false;
  state.set_team(0, team0);
  state.set_team(1, team1);
  state.startBattle();
  // Teams should be locked
  Team &newTeam0 = state.teams[0];
  if (!newTeam0.lock || !state.teams[1].lock)
    return false;
  if (!newTeam0.pkmn[0].lock)
    return false;
  // The team in the state should be a *copy* of the team generated.
  // Original isn't locked: can add.
  if (!team0.add_pokemon(0, charmander))
    return false;
  // Copy in state is locked: cannot add.
  if (newTeam0.add_pokemon(0, charmander))
    return false;
  if (newTeam0.pokemonLeft != 2)
    return false;
  if (newTeam0.activeInd != 0)
    return false;
  state.set_choices();
  for (int moveInd = 0; moveInd < 4; moveInd++) {
    // We've given Charmander exactly two moves
    if (moveInd < 2 && !newTeam0.choicesAvailable.move[moveInd])
      return false;
    // so they shouldn't get moves at indices 2 or 3
    if (moveInd >= 2 && newTeam0.choicesAvailable.move[moveInd])
      return false;
  }
  for (int switchInd = 0; switchInd < 6; switchInd++) {
    // Charmander can switch to exactly Bulbasaur in slot 1.
    if (switchInd != 1 && newTeam0.choicesAvailable.swap[switchInd])
      return false;
    if (switchInd == 1 && !newTeam0.choicesAvailable.swap[switchInd])
      return false;
  }
  // The battle shouldn't have ended yet: nothing happened.
  if (state.checkWin() >= 0) {
    return false;
  }
  return true;
}
bool test_basic_damage() {
  // Neither Lillipup nor Tackle are changed in Run and Bun
  Pokemon attacker = Pokemon(PokeName::LILLIPUP, 10, Gender::MALE, Nature::HARDY, Stats{}, Stats{});
  Pokemon defender = Pokemon(PokeName::LILLIPUP, 5, Gender::MALE, Nature::HARDY, Stats{}, Stats{});
  attacker.set_ability(Ability::VITAL_SPIRIT);
  defender.set_ability(Ability::VITAL_SPIRIT);
  attacker.add_move(MoveId::TACKLE, 0);
  defender.add_move(MoveId::TACKLE, 0);
  Team team0, team1;
  team0.add_pokemon(0, attacker);
  team1.add_pokemon(0, defender);
  BattleState state;
  state.set_team(0, team0);
  state.set_team(1, team1);
  state.startBattle();
  // Check the max rolled damage
  MoveInstance tackleInstance = MoveInstance{moveDict.dict[MoveId::TACKLE]};
  auto dmg = state
                 .getDamage(attacker, defender, tackleInstance,
                            DMGCalcOptions{.roll_val = 15, .crit = false})
                 .damageDealt;
  if (dmg != 16) {
    std::cout << "Max roll, no crit dmg: " << dmg << std::endl;
    return false; // From Showdown calculator
  }
  // Check min rolled damage with crit
  tackleInstance = MoveInstance{moveDict.dict[MoveId::TACKLE]};
  dmg = state
            .getDamage(attacker, defender, tackleInstance,
                       DMGCalcOptions{.roll_val = 0, .crit = true})
            .damageDealt;
  if (dmg != 19) {
    std::cout << "Min roll with crit dmg: " << dmg << std::endl;
    return false;
  }
  return true;
}
BattleState get_basic_battle(DMGCalcOptions ddo) {
  Pokemon charmander =
      Pokemon(PokeName::CHARMANDER, 5, Gender::MALE, Nature::HARDY, Stats{31,31,31,31,31,31}, Stats{});
  Pokemon bulbasaur =
      Pokemon(PokeName::BULBASAUR, 5, Gender::MALE, Nature::HARDY, Stats{31,31,31,31,31,31}, Stats{});
  charmander.set_ability(Ability::BLAZE);
  bulbasaur.set_ability(Ability::OVERGROW);
  charmander.add_move(MoveId::SCRATCH, 0);
  charmander.add_move(MoveId::EMBER, 1);
  bulbasaur.add_move(MoveId::TACKLE, 0);
  Team team0, team1;
  team0.add_pokemon(0, charmander);
  team0.add_pokemon(1, bulbasaur);
  team1.add_pokemon(0, bulbasaur);
  BattleState state{ddo};
  state.set_team(0, team0);
  state.set_team(1, team1);
  state.startBattle();
  return state;
}
bool test_basic_moves() {
  // TEST RAW DAMAGE
  // Min roll Scratch does 4 dmg
  BattleState state = get_basic_battle({0, 0});
  Pokemon &attacker = state.getActivePokemon(0);
  Pokemon &defender = state.getActivePokemon(1);
  if(defender.current_hp != 21) return false;
  state.runMove(attacker.moves[0], attacker, defender);
  if(defender.current_hp != 17) return false;
  // Max roll Scratch does 5 dmg
  state = get_basic_battle({15, 0});
  attacker = state.getActivePokemon(0);
  defender = state.getActivePokemon(1);
  if(defender.current_hp != 21) return false;
  state.runMove(attacker.moves[0], attacker, defender);
  if(defender.current_hp != 16) return false;
  // Max roll crit Tackle does 7 dmg
  state = get_basic_battle({15, 4});
  attacker = state.getActivePokemon(0);
  defender = state.getActivePokemon(1);
  if(defender.current_hp != 21) return false;
  TIME_EXPR("Charmander attack", state.runMove(attacker.moves[0], attacker, defender););
  if(defender.current_hp != 14) return false;
  std::swap(attacker, defender);
  if(defender.current_hp != 20) return false;
  TIME_EXPR("Bulbasaur attack", state.runMove(attacker.moves[0], attacker, defender););
  if(defender.current_hp != 13) return false;
  // TEST STATUS MOVE
  return true;
}
} // namespace pkmn
std::vector<testing::TestCase> battle_tests = {
    {"Testing starting a battle", pkmn::test_initialize_battle},
    {"Very basic attack damage test", pkmn::test_basic_damage},
    {"Testing the simplest of moves", pkmn::test_basic_moves},
};