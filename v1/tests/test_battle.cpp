#include "battle_state.hpp"
#include "test_framework.hpp"
#include <chrono>
#include <iostream>

#define TIME_EXPR(label, code)                                                                     \
  do {                                                                                             \
    auto start = std::chrono::high_resolution_clock::now();                                        \
    code auto end = std::chrono::high_resolution_clock::now();                                     \
    std::chrono::duration<double, std::milli> elapsed = end - start;                               \
    std::cout << label << ": " << elapsed.count() << " ms\n";                                      \
  } while (0)
#define TIME_EXPR_RET(label, expr)                                                                 \
  [&]() {                                                                                          \
    auto start = std::chrono::high_resolution_clock::now();                                        \
    auto result = (expr);                                                                          \
    auto end = std::chrono::high_resolution_clock::now();                                          \
    std::chrono::duration<double, std::milli> elapsed = end - start;                               \
    std::cout << label << ": " << elapsed.count() << " ms\n";                                      \
    return result;                                                                                 \
  }()
namespace pkmn {
Pokemon getCharmander(int lvl) {
  Pokemon charmander = Pokemon(PokeName::CHARMANDER, lvl, Gender::MALE, Nature::HARDY,
                               Stats{31, 31, 31, 31, 31, 31}, Stats{});
  charmander.set_ability(Ability::BLAZE);
  charmander.add_move(MoveId::SCRATCH, 0);
  charmander.add_move(MoveId::EMBER, 1);
  charmander.add_move(MoveId::WILLOWISP, 2);
  return charmander;
}
Pokemon getBulbasaur(int lvl) {
  Pokemon bulbasaur = Pokemon(PokeName::BULBASAUR, lvl, Gender::MALE, Nature::HARDY,
                              Stats{31, 31, 31, 31, 31, 31}, Stats{});
  bulbasaur.set_ability(Ability::OVERGROW);
  bulbasaur.add_move(MoveId::TACKLE, 0);
  return bulbasaur;
}
// Test starting a battle
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
// Test very simple getDamage() calls
bool test_basic_damage() {
  // Neither Lillipup nor Tackle are changed in Run and Bun
  Pokemon attacker = Pokemon(PokeName::LILLIPUP, 10, Gender::MALE, Nature::HARDY, Stats{31, 31, 31, 31, 31, 31}, Stats{});
  Pokemon defender = Pokemon(PokeName::LILLIPUP, 5, Gender::MALE, Nature::HARDY, Stats{31, 31, 31, 31, 31, 31}, Stats{});
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
                 .getDamage(state.getActivePokemon(0), state.getActivePokemon(1), tackleInstance,
                            DMGCalcOptions{.roll_val = 15, .crit = false})
                 .damageDealt;
  if (dmg != 15) {
    std::cout << "Max roll, no crit dmg: " << dmg << std::endl;
    return false; // From Showdown calculator
  }
  // Check min rolled damage with crit
  tackleInstance = MoveInstance{moveDict.dict[MoveId::TACKLE]};
  dmg = state
            .getDamage(state.getActivePokemon(0), state.getActivePokemon(1), tackleInstance,
                       DMGCalcOptions{.roll_val = 0, .crit = 4})
            .damageDealt;
  if (dmg != 18) {
    std::cout << "Min roll with crit dmg: " << dmg << std::endl;
    return false;
  }
  return true;
}
// Test Tackle and Ember for damage and moveHit(Will-O-Wisp) for status
bool test_basic_moves() {
  BattleState state;
  Team team0, team1;
  team0.add_pokemon(0, getCharmander(5));
  team0.add_pokemon(1, getBulbasaur(5));
  team1.add_pokemon(0, getBulbasaur(5));
  Pokemon attacker, defender;
  // TEST RAW DAMAGE
  // Min rolls 
  {
    // Scratch does 4 dmg
    state = {{0, 0}};
    state.set_team(0, team0), state.set_team(1, team1);
    state.startBattle();
    BattleState startState = state;
    attacker = state.getActivePokemon(0);
    defender = state.getActivePokemon(1);
    if (defender.current_hp != 21)
      return false;
    TIME_EXPR("Min-roll no-crit Scratch", state.runMove(attacker.moves[0], attacker, defender););
    if (defender.current_hp != 17)
      return false;
    // Ember does 8 dmg
    state = startState;
    attacker = state.getActivePokemon(0);
    defender = state.getActivePokemon(1);
    if (defender.current_hp != 21)
      return false;
    TIME_EXPR("Min-roll no-crit Ember", state.runMove(attacker.moves[1], attacker, defender););
    if (defender.current_hp != 13)
      return false;
  }
  // Max roll Scratch does 5 dmg
  {
    state = {{15, 0}};
    state.set_team(0, team0), state.set_team(1, team1);
    state.startBattle();
    attacker = state.getActivePokemon(0);
    defender = state.getActivePokemon(1);
    if (defender.current_hp != 21)
      return false;
    TIME_EXPR("Max-roll no-crit Scratch", state.runMove(attacker.moves[0], attacker, defender););
    if (defender.current_hp != 16)
      return false;
  }
  // Max roll crit Tackle does 7 dmg
  {
    state = {{15, 4}};
    state.set_team(0, team0), state.set_team(1, team1);
    state.startBattle();
    attacker = state.getActivePokemon(0);
    defender = state.getActivePokemon(1);
    if (defender.current_hp != 21)
      return false;
    TIME_EXPR("Max-roll crit Scratch", state.runMove(attacker.moves[0], attacker, defender););
    if (defender.current_hp != 14)
      return false;
    std::swap(attacker, defender);
    if (defender.current_hp != 20)
      return false;
    TIME_EXPR("Max-roll crit Tackle", state.runMove(attacker.moves[0], attacker, defender););
    if (defender.current_hp != 13)
      return false;
  }
  // TEST STATUS MOVE
  {
    state = {{0, 0}};
    state.set_team(0, team0), state.set_team(1, team1);
    state.startBattle();
    attacker = state.getActivePokemon(0);
    defender = state.getActivePokemon(1);
    // std::cout << "Initial status: " << (int)defender.status << std::endl;
    if (defender.status != Status::NO_STATUS)
      return false;
    MoveInstance moveInst(attacker.moves[2].id);
    // std::cout << (int)moveInst.moveData.id << std::endl; // 836 Will-O-Wisp
    TIME_EXPR("moveHit Will-O-Wisp", state.moveHit(defender, attacker, moveInst););
    // std::cout << "Final status: " << (int)defender.status << std::endl;
    if (defender.status != Status::BURN)
      return false;
  }
  return true;
}
// Test setting/reading move/swap choices
bool test_options() {
  BattleState state;
  Team team0, team1;
  team0.add_pokemon(0, getCharmander(5));
  team0.add_pokemon(1, getBulbasaur(5));
  team1.add_pokemon(0, getBulbasaur(100));
  state = {{0, 0}};
  state.set_team(0, team0), state.set_team(1, team1);
  TIME_EXPR("startBattle", state.startBattle(););
  // Test initial switch and move options
  {
    Choice &choice0 = state.teams[0].choicesAvailable;
    if (state.turnNum != 1)
      return false;
    // Can only swap to slot 1
    for (int i = 0; i < 6; i++) {
      if ((i == 1) ^ choice0.swap[i])
        return false;
    }
    // Has three moves to select from
    for (int i = 0; i < 4; i++) {
      if ((i < 3) ^ choice0.move[i])
        return false;
    }
    choice0 = Choice(false, 0, -1);
    Choice &choice1 = state.teams[0].choicesAvailable;
    // No ally to swap to
    for (int i = 0; i < 6; i++) {
      if (choice1.swap[i])
        return false;
    }
    // Can only select moveslot 0
    for (int i = 0; i < 4; i++) {
      if ((i == 0) ^ choice1.move[i])
        return false;
    }
    choice1 = Choice(false, 0, -1); // Select moveslot 0
  }
  state = TIME_EXPR_RET("Run turn loop", state.runTurnPy());
  // Test instaswitch flag(s) and switch options
  {
    if (state.turnNum != 1)
      return false;
    // Team 0 fainted, must switch
    if (!state.teams[0].instaSwitch)
      return false;
    // Team 1 cannot update choice
    if (state.teams[1].instaSwitch)
      return false;
    Choice &choice0 = state.teams[0].choicesAvailable;
    // Can only switch to slot 1
    for (int i = 0; i < 6; i++) {
      // std::cout << i << ' ' << choice.swap[i] << std::endl;
      if ((i == 1) ^ choice0.swap[i])
        return false;
    }
    // Moves are unavailable
    for (int i = 0; i < 4; i++) {
      if (choice0.move[i])
        return false;
    } // No need to do anything to select swap to slot 1
    // team 1 should have had choices cleared after moving
    Choice &choice1 = state.teams[1].choicesAvailable;
    for (int i = 0; i < 6; i++) {
      if (choice1.swap[i])
        return false;
    }
    for (int i = 0; i < 4; i++) {
      if (choice1.move[i])
        return false;
    }
  }
  state = TIME_EXPR_RET("Run turn loop", state.runTurnPy());
  // New turn should have started
  if (state.turnNum != 2)
    return false;
  // Team 1 should have swapped in Bulbasaur
  if (state.getActivePokemon(0).species != Species::BULBASAUR)
    return false;
  return true;
}
bool test_turn() {
  BattleState state;
  Team team0, team1;
  // Ember vs. Tackle
  {
    team0.add_pokemon(0, getCharmander(5));
    team1.add_pokemon(0, getBulbasaur(5));
    state = {{0, 0}};
    state.set_team(0, team0), state.set_team(1, team1);
    state.startBattle();
    if(state.getActivePokemon(0).current_hp != 20) return false;
    if(state.getActivePokemon(1).current_hp != 21) return false;
    state.teams[0].choicesAvailable = Choice(false, 1, -1);
    state.teams[1].choicesAvailable = Choice(false, 0, -1);
    state = TIME_EXPR_RET("Run turn loop", state.runTurnPy());
    if(state.getActivePokemon(0).current_hp != 16) return false;
    if(state.getActivePokemon(1).current_hp != 13) return false;
  }
  return true;
}
bool test_random() {
  int cnt = 0;
  int NUM_TRIALS = 1000;
  int numer = 1;
  int denom = 4;
  for(int i=0; i<NUM_TRIALS; i++) {
    if(math::randomChance(numer, denom)) cnt++;
  }
  std::cout << "Intended: " << numer << "/" << denom << std::endl;
  std::cout << "Actual: " << cnt << "/" << NUM_TRIALS << std::endl;
  return true;
}
} // namespace pkmn
std::vector<testing::TestCase> battle_tests = {
    {"Starting a battle", pkmn::test_initialize_battle},
    {"Very simple getDamage() calls", pkmn::test_basic_damage},
    {"Tackle for damage and moveHit(Will-O-Wisp) for status", pkmn::test_basic_moves},
    {"Setting/reading move/swap choices", pkmn::test_options},
    {"A couple example turns", pkmn::test_turn},
    {"Randomness util", pkmn::test_random},
};