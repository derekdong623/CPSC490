#include "battle_state.hpp"
#include "test_framework.hpp"
#include <chrono>
#include <cmath>
#include <iostream>

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
Pokemon getSquirtle(int lvl) {
  Pokemon squirtle = Pokemon(PokeName::SQUIRTLE, lvl, Gender::MALE, Nature::HARDY,
                             Stats{31, 31, 31, 31, 31, 31}, Stats{});
  squirtle.set_ability(Ability::RAIN_DISH);
  squirtle.add_move(MoveId::AQUATAIL, 0);
  return squirtle;
}
Pokemon getGolem(int lvl) {
  Pokemon golem = Pokemon(PokeName::GOLEM, lvl, Gender::MALE, Nature::HARDY,
                          Stats{31, 31, 31, 31, 31, 31}, Stats{});
  golem.set_ability(Ability::STURDY);
  golem.add_move(MoveId::EXPLOSION, 0);
  return golem;
}
Pokemon getPiplup(int lvl, Item item) {
  Pokemon piplup = Pokemon(PokeName::PIPLUP, lvl, Gender::MALE, Nature::HARDY,
                           Stats{31, 31, 31, 31, 31, 31}, Stats{});
  piplup.set_ability(Ability::DEFIANT);
  piplup.set_item(item);
  piplup.add_move(MoveId::POUND, 0);
  piplup.add_move(MoveId::GROWL, 1);
  piplup.add_move(MoveId::BUBBLE, 2);
  piplup.add_move(MoveId::PLUCK, 3);
  return piplup;
}
Pokemon getPoochyena(int lvl, Item item) {
  Pokemon poochyena = Pokemon(PokeName::POOCHYENA, lvl, Gender::MALE, Nature::HARDY,
                              Stats{31, 31, 31, 31, 31, 31}, Stats{});
  poochyena.set_ability(Ability::RATTLED);
  poochyena.set_item(item);
  poochyena.add_move(MoveId::BITE, 0);
  poochyena.add_move(MoveId::QUICKATTACK, 1);
  poochyena.add_move(MoveId::SANDATTACK, 2);
  return poochyena;
}
Pokemon getLillipup(int lvl, Item item) {
  Pokemon lillipup = Pokemon(PokeName::LILLIPUP, lvl, Gender::MALE, Nature::HARDY,
                             Stats{31, 31, 31, 31, 31, 31}, Stats{});
  lillipup.set_ability(Ability::VITAL_SPIRIT);
  lillipup.set_item(item);
  lillipup.add_move(MoveId::TACKLE, 0);
  lillipup.add_move(MoveId::BITE, 1);
  lillipup.add_move(MoveId::SANDATTACK, 2);
  return lillipup;
}
Pokemon getRookidee(int lvl, Item item) {
  Pokemon rookidee = Pokemon(PokeName::ROOKIDEE, lvl, Gender::MALE, Nature::HARDY,
                             Stats{31, 31, 31, 31, 31, 31}, Stats{});
  rookidee.set_ability(Ability::KEEN_EYE);
  rookidee.set_item(item);
  rookidee.add_move(MoveId::WINGATTACK, 0);
  rookidee.add_move(MoveId::SANDATTACK, 1);
  rookidee.add_move(MoveId::SWAGGER, 2);
  return rookidee;
}
Pokemon getCroagunk(int lvl, Item item) {
  Pokemon croagunk = Pokemon(PokeName::CROAGUNK, lvl, Gender::MALE, Nature::HARDY,
                             Stats{31, 31, 31, 31, 31, 31}, Stats{});
  croagunk.set_ability(Ability::ANTICIPATION);
  croagunk.set_item(item);
  croagunk.add_move(MoveId::POISONJAB, 0);
  croagunk.add_move(MoveId::BELCH, 1);
  croagunk.add_move(MoveId::CALMMIND, 2);
  return croagunk;
}
Pokemon getExeggcute(int lvl, Item item) {
  Pokemon exeggcute = Pokemon(PokeName::EXEGGCUTE, lvl, Gender::MALE, Nature::HARDY,
                              Stats{31, 31, 31, 31, 31, 31}, Stats{});
  exeggcute.set_ability(Ability::HARVEST);
  exeggcute.set_item(item);
  exeggcute.add_move(MoveId::CONFUSION, 0);
  exeggcute.add_move(MoveId::BULLETSEED, 1);
  exeggcute.add_move(MoveId::LEECHSEED, 2);
  exeggcute.add_move(MoveId::STUNSPORE, 3);
  return exeggcute;
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
  Pokemon attacker = Pokemon(PokeName::LILLIPUP, 10, Gender::MALE, Nature::HARDY,
                             Stats{31, 31, 31, 31, 31, 31}, Stats{});
  Pokemon defender = Pokemon(PokeName::LILLIPUP, 5, Gender::MALE, Nature::HARDY,
                             Stats{31, 31, 31, 31, 31, 31}, Stats{});
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
    if (state.getActivePokemon(0).current_hp != 20)
      return false;
    if (state.getActivePokemon(1).current_hp != 21)
      return false;
    state.teams[0].choicesAvailable = Choice(false, 1, -1);
    state.teams[1].choicesAvailable = Choice(false, 0, -1);
    state = TIME_EXPR_RET("Run turn loop", state.runTurnPy());
    if (state.getActivePokemon(0).current_hp != 16)
      return false;
    if (state.getActivePokemon(1).current_hp != 13)
      return false;
  }
  return true;
}
bool test_random() {
  int cnt = 0;
  int NUM_TRIALS = 1000;
  int numer = 1;
  int denom = 4;
  for (int i = 0; i < NUM_TRIALS; i++) {
    if (math::randomChance(numer, denom))
      cnt++;
  }
  std::cout << "Intended: " << numer << "/" << denom << std::endl;
  std::cout << "Actual: " << cnt << "/" << NUM_TRIALS << std::endl;
  return true;
}
void process_timings(std::string label, std::vector<double> times, int num_trials) {
  double meanTime, varTime;
  meanTime = 0.;
  for (int i = 0; i < num_trials; i++) {
    meanTime += times[i];
  }
  meanTime /= num_trials;
  varTime = 0.;
  for (int i = 0; i < num_trials; i++) {
    varTime += (times[i] - meanTime) * (times[i] - meanTime);
  }
  varTime /= num_trials;
  std::cout << label << meanTime << " +/- " << 2 * std::sqrt(varTime) << std::endl;
}
bool test_timing() {
  BattleState state;
  Team team0, team1;
  int NUM_TRIALS = 1000;
  { // damage
    team0.add_pokemon(0, getCharmander(5));
    team1.add_pokemon(0, getBulbasaur(5));
    state = {{0, 0}};
    state.set_team(0, team0), state.set_team(1, team1);
    BattleState startState = state; // Should copy
    std::vector<double> startTime, turnTime;
    for (int i = 0; i < NUM_TRIALS; i++) {
      double timeTaken = 0;
      timeTaken = TIME_EXPR_TIMING(state.startBattle(););
      startTime.push_back(timeTaken);
      state.teams[0].choicesAvailable = Choice(false, 1, -1);
      state.teams[1].choicesAvailable = Choice(false, 0, -1);
      timeTaken = TIME_EXPR_TIMING(state = state.runTurnPy(););
      turnTime.push_back(timeTaken);
      state = startState;
    }
    process_timings("start action time: ", startTime, NUM_TRIALS);
    process_timings("turn time: ", turnTime, NUM_TRIALS);
  }
  { // switch
    team0.add_pokemon(0, getCharmander(5));
    team0.add_pokemon(1, getCharmander(5));
    team1.add_pokemon(0, getBulbasaur(100));
    state = {{0, 0}};
    state.set_team(0, team0), state.set_team(1, team1);
    state.teams[0].choicesAvailable = Choice(false, 1, -1);
    state.teams[1].choicesAvailable = Choice(false, 0, -1);
    state = state.runTurnPy();
    state.teams[0].choicesAvailable = Choice(false, -1, 1);
    BattleState startState = state; // Should copy
    std::vector<double> switchTime;
    for (int i = 0; i < NUM_TRIALS; i++) {
      double timeTaken = 0;
      timeTaken = TIME_EXPR_TIMING(state = state.runTurnPy(););
      switchTime.push_back(timeTaken);
      state = startState;
    }
    process_timings("switch time: ", switchTime, NUM_TRIALS);
  }
  { // switch
    team0.add_pokemon(0, getCharmander(5));
    team0.add_pokemon(1, getCharmander(5));
    team1.add_pokemon(0, getGolem(100));
    team1.add_pokemon(1, getCharmander(5));
    state = {{0, 0}};
    state.set_team(0, team0), state.set_team(1, team1);
    state.teams[0].choicesAvailable = Choice(false, 1, -1);
    state.teams[1].choicesAvailable = Choice(false, 0, -1);
    state = state.runTurnPy();
    state.teams[0].choicesAvailable = Choice(false, -1, 1);
    state.teams[1].choicesAvailable = Choice(false, -1, 1);
    BattleState startState = state; // Should copy
    std::vector<double> switchTime;
    for (int i = 0; i < NUM_TRIALS; i++) {
      double timeTaken = 0;
      timeTaken = TIME_EXPR_TIMING(state = state.runTurnPy(););
      switchTime.push_back(timeTaken);
      state = startState;
    }
    process_timings("double switch time: ", switchTime, NUM_TRIALS);
  }
  return true;
}
bool test_priority() {
  BattleState state;
  Team team0, team1;
  team0.add_pokemon(0, getPiplup(10, Item::NO_ITEM));
  team1.add_pokemon(0, getPoochyena(5, Item::NO_ITEM));
  state.set_team(0, team0);
  state.set_team(1, team1);
  state.startBattle();
  state.teams[0].choicesAvailable = Choice(false, 1, -1); // Growl
  state.teams[1].choicesAvailable = Choice(false, 1, -1); // Quick Attack
  state = state.runTurnPy();
  return true;
}
bool test_flinch() {
  BattleState state;
  Team team0, team1;
  team0.add_pokemon(0, getPiplup(9, Item::NO_ITEM));
  team1.add_pokemon(0, getPoochyena(10, Item::NO_ITEM)); // Pooch is faster
  state.set_team(0, team0);
  state.set_team(1, team1);
  state.secondaryDenom = 1; // Guarantee secondary
  state.startBattle();
  state.teams[0].choicesAvailable = Choice(false, 0, -1); // Pound
  state.teams[1].choicesAvailable = Choice(false, 0, -1); // Bite
  state = state.runTurnPy();
  // Piplup shouldn't get to do damage
  if (state.getActivePokemon(1).current_hp != state.getActivePokemon(1).stats.hp) {
    return false;
  }
  // But the volatile shouldn't last
  if (state.getActivePokemon(0).flinch) {
    return false;
  }
  return true;
}
bool test_accuracy() {
  BattleState state, startState;
  Team team0, team1;
  int NUM_TRIALS = 100;
  int num_hit = 0;
  { // Move-inherent inaccuracy
    team0.add_pokemon(0, getPiplup(50, Item::NO_ITEM));
    team1.add_pokemon(0, getSquirtle(50));
    state.set_team(0, team0);
    state.set_team(1, team1);
    state.startBattle();
    startState = state;
    for (int i = 0; i < NUM_TRIALS; i++) {
      state.teams[0].choicesAvailable = Choice(false, 1, -1); // Growl
      state.teams[1].choicesAvailable = Choice(false, 0, -1); // Aqua Tail
      state = state.runTurnPy();
      if (state.getActivePokemon(0).current_hp < state.getActivePokemon(0).stats.hp) {
        num_hit++;
      }
      state = startState;
    }
    std::cout << "Accuracy is 90. Num hit: " << num_hit << " of " << NUM_TRIALS << std::endl;
  }
  { // Accuracy drop
    num_hit = 0;
    state = {};
    team0 = {}, team1 = {};
    team0.add_pokemon(0, getPiplup(50, Item::NO_ITEM));
    team1.add_pokemon(0, getPoochyena(50, Item::NO_ITEM));
    state.set_team(0, team0);
    state.set_team(1, team1);
    state.startBattle();
    state.startBattle();
    startState = state;
    for (int i = 0; i < NUM_TRIALS; i++) {
      state.teams[0].choicesAvailable = Choice(false, 1, -1); // Growl
      state.teams[1].choicesAvailable = Choice(false, 2, -1); // Sand Attack
      state = state.runTurnPy();
      if (state.getActivePokemon(0).boosts[ModifierId::ACCURACY] != -1)
        return false;
      state.teams[0].choicesAvailable = Choice(false, 0, -1); // Pound
      state.teams[1].choicesAvailable = Choice(false, 1, -1); // Quick Attack
      state = state.runTurnPy();
      if (state.getActivePokemon(1).current_hp < state.getActivePokemon(1).stats.hp) {
        num_hit++;
      }
      state = startState;
    }
    std::cout << "One accuracy drop, should be 75%. Num hit: " << num_hit << " of " << NUM_TRIALS
              << std::endl;
  }
  return true;
}
bool test_stat_boost() {
  BattleState state;
  Team team0, team1;
  team0.add_pokemon(0, getPiplup(10, Item::NO_ITEM));
  team1.add_pokemon(0, getPoochyena(10, Item::NO_ITEM));
  state = {{0, 0}}; // Min-roll, no-crit
  state.set_team(0, team0);
  state.set_team(1, team1);
  state.startBattle();
  BattleState startState = state;
  { // Check damage given boost
    // Test stat boosted damage
    if (state.getActivePokemon(1).current_hp != 30)
      return false;
    state.getActivePokemon(0).boosts[ModifierId::ATTACK] = 1;
    state.teams[0].choicesAvailable = Choice(false, 0, -1); // Pound
    state.teams[1].choicesAvailable = Choice(false, 0, -1); // Bite
    state = state.runTurnPy();
    if (state.getActivePokemon(1).current_hp != 22) {
      return false;
    }
    // Test stat unboosted damage
    state.getActivePokemon(0).boosts[ModifierId::ATTACK] = -1;
    state.teams[0].choicesAvailable = Choice(false, 0, -1); // Pound
    state.teams[1].choicesAvailable = Choice(false, 0, -1); // Bite
    state = state.runTurnPy();
    if (state.getActivePokemon(1).current_hp != 18)
      return false;
  }
  { // Check boost applied
    state = startState;
    state.teams[0].choicesAvailable = Choice(false, 1, -1); // Growl
    state.teams[1].choicesAvailable = Choice(false, 0, -1); // Bite
    state = state.runTurnPy();
    if (state.getActivePokemon(1).boosts[ModifierId::ATTACK] != -1)
      return false;
  }
  return true;
}
bool test_confusion() {
  BattleState state;
  Team team0, team1;
  team0.add_pokemon(0, getPiplup(10, Item::NO_ITEM));
  team1.add_pokemon(0, getRookidee(10, Item::NO_ITEM)); // Rook is faster
  team1.add_pokemon(1, getRookidee(10, Item::NO_ITEM));
  state.set_team(0, team0);
  state.set_team(1, team1);
  state.guaranteeHit = true;
  state.startBattle();
  {
    state.teams[0].choicesAvailable = Choice(false, 0, -1); // Pound
    state.teams[1].choicesAvailable = Choice(false, 2, -1); // Swagger
    state = state.runTurnPy();
    if (state.getActivePokemon(0).boosts[ModifierId::ATTACK] != 2)
      return false;
    int confusionTurnsLeft = state.getActivePokemon(0).confusion;
    if (!confusionTurnsLeft)
      return false;
    state.teams[0].choicesAvailable = Choice(false, 0, -1); // Pound
    state.teams[1].choicesAvailable = Choice(false, -1, 1); // switch
    state = state.runTurnPy();
    if (state.getActivePokemon(0).confusion != confusionTurnsLeft - 1)
      return false;
    // Manually checked that the confusion damage is correct
  }
  return true;
}
bool test_berry() {
  BattleState state;
  Team team0, team1;
  team0.add_pokemon(0, getPiplup(10, Item::ORAN_BERRY));
  team1.add_pokemon(0, getRookidee(10, Item::ORAN_BERRY)); // Rook is faster
  state = {{0, 0}};
  state.set_team(0, team0);
  state.set_team(1, team1);
  state.startBattle();
  BattleState startState = state;
  {                                              // Oran Berry
    Pokemon &piplup = state.getActivePokemon(0); // MAX HP: 33
    piplup.current_hp = 20;
    state.teams[0].choicesAvailable = Choice(false, 1, -1); // Growl
    state.teams[1].choicesAvailable = Choice(false, 0, -1); // WingAttack
    state = state.runTurnPy();
    // WingAttack does 9, OranBerry heals 10
    if (state.getActivePokemon(0).current_hp != 21)
      return false;
  }
  { // Pluck
    state = startState;
    Pokemon &piplup = state.getActivePokemon(0); // MAX HP: 33
    piplup.current_hp = 20;
    state.teams[0].choicesAvailable = Choice(false, 3, -1); // Pluck
    state.teams[1].choicesAvailable = Choice(false, 0, -1); // WingAttack
    state = state.runTurnPy();
    // Rookidee WingAttack does 9, OranBerry heals 10
    // Piplup Pluck does dmg and heals it with OranBerry for 10
    if (state.getActivePokemon(0).current_hp != 31)
      return false;
  }
  return true;
}
bool test_disable() {
  BattleState state{{0, 0}};
  Team team0, team1;
  team0.add_pokemon(0, getPiplup(10, Item::NO_ITEM));
  team1.add_pokemon(0, getCroagunk(10, Item::ORAN_BERRY)); // starts with 32 HP
  state.set_team(0, team0);
  state.set_team(1, team1);
  state.startBattle();
  {
    for (int i = 0; i < 4; i++) {
      if (state.teams[1].choicesAvailable.move[1]) {
        // Belch should be disabled
        return false;
      }
      state.teams[0].choicesAvailable = Choice(false, 0, -1); // Pound
      state.teams[1].choicesAvailable = Choice(false, 2, -1); // CalmMind
      // Pound does 5 damage
      state = state.runTurnPy();
    }
    if (state.getActivePokemon(1).current_hp != 22) {
      return false;
    }
    if (!state.getActivePokemon(1).ateBerry) {
      return false;
    }
    if (!state.teams[1].choicesAvailable.move[1]) {
      // Belch should be enabled after eating a berry
      return false;
    }
  }
  return true;
}
bool test_multihit() {
  BattleState state{{0, 0}};
  Team team0, team1;
  int NUM_TRIALS = 100;
  int num_hit = 0;
  team0.add_pokemon(0, getLillipup(10, Item::NO_ITEM)); // starts with 32 HP
  team1.add_pokemon(0, getExeggcute(10, Item::NO_ITEM));
  state.set_team(0, team0);
  state.set_team(1, team1);
  state.startBattle();
  BattleState startState = state;
  for (int i = 0; i < NUM_TRIALS; i++, state = startState) {
    state.teams[0].choicesAvailable = Choice(false, 0, -1); // Tackle
    state.teams[1].choicesAvailable = Choice(false, 1, -1); // BulletSeed
    state = state.runTurnPy();
    // 2-5 BulletSeed hits, each does 4 dmg
    int lillipupHP = state.getActivePokemon(0).current_hp;
    if ((32 - lillipupHP) % 4) {
      return false;
    }
    num_hit += (32 - lillipupHP) / 4;
  }
  std::cout << "Hit: " << num_hit << " in trials: " << NUM_TRIALS << std::endl;
  return true;
}

Pokemon getCarvanha() {
  Pokemon carvanha = Pokemon(PokeName::CARVANHA, 11, Gender::MALE, Nature::NAIVE,
                             Stats{31, 31, 31, 31, 31, 31}, Stats{});
  carvanha.set_ability(Ability::HARVEST);
  carvanha.set_item(Item::ORAN_BERRY);
  carvanha.add_move(MoveId::BITE, 0);
  carvanha.add_move(MoveId::WATERPULSE, 1);
  carvanha.add_move(MoveId::AQUAJET, 2);
  carvanha.add_move(MoveId::POISONFANG, 3);
  return carvanha;
}
Pokemon getAquaCroagunk() {
  Pokemon croagunk = Pokemon(PokeName::CROAGUNK, 12, Gender::MALE, Nature::HASTY,
                             Stats{31, 31, 31, 31, 31, 31}, Stats{});
  croagunk.set_ability(Ability::POISON_TOUCH);
  croagunk.set_item(Item::SALAC_BERRY);
  croagunk.add_move(MoveId::BELCH, 0);
  croagunk.add_move(MoveId::ROCKSMASH, 1);
  croagunk.add_move(MoveId::POISONSTING, 2);
  croagunk.add_move(MoveId::FAKEOUT, 3);
  return croagunk;
}

Pokemon getParas() {
  Pokemon paras =
      Pokemon(PokeName::PARAS, 12, Gender::FEMALE, Nature::MILD, Stats{0, 5, 2, 16, 1, 1}, Stats{});
  paras.set_ability(Ability::EFFECT_SPORE);
  // paras.set_item(Item::ORAN_BERRY);
  paras.add_move(MoveId::BUGBITE, 0);
  paras.add_move(MoveId::ABSORB, 1);
  paras.add_move(MoveId::STUNSPORE, 2);
  paras.add_move(MoveId::POISONPOWDER, 3);
  return paras;
}

bool test_aqua() {
  BattleState state{{0,0}};
  Team team0, team1;
  team0.add_pokemon(0, getParas()); // Start at 30 HP
  team1.add_pokemon(0, getAquaCroagunk()); // Start at 37 HP
  state.set_team(0, team0);
  state.set_team(1, team1);
  state.startBattle();
  state.teams[0].choicesAvailable = Choice(false, 0, -1); // BugBite
  state.fillOpponentChoice(1);
  state = state.runTurnPy();
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

    // {"Timing", pkmn::test_timing},
    // {"Priority moves", pkmn::test_priority}, // Comment other tests if included
    {"Flinch", pkmn::test_flinch},
    {"Accuracy", pkmn::test_accuracy},
    {"Stat changes", pkmn::test_stat_boost},
    {"Confusion", pkmn::test_confusion},
    {"Berry mechanics", pkmn::test_berry},
    {"Disabling move", pkmn::test_disable},
    {"Multihit moves", pkmn::test_multihit},
    // {"Temp Aqua Grunt tests", pkmn::test_aqua},
};