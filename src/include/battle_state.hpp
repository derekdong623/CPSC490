#pragma once

#include "move_instance.hpp"
#include "utils.hpp"
#include <array>
#include <iostream>
#include <map>
#include <optional>
#include <queue>
namespace pkmn {
// Used to sort which side moves first
struct MoveActionPriority {
  int priority = -1; // Lower first
  int fractionalPriority = -1;
  int speed = -1;
  int side = -1;
  int moveInd = -1;
  // Action order should be *decreasing* priority, fractionalPriority, and speed.
  bool operator<(const MoveActionPriority &other) const {
    return std::tie(priority, fractionalPriority, speed) >
           std::tie(other.priority, other.fractionalPriority, other.speed);
  }
};
// Used to sort which switch happens first
struct SwitchAction {
  bool instaswitch = false;
  int speed = -1;
  int side = -1;
  int newPokeInd = -1;
  bool operator<(const SwitchAction &other) const {
    return std::make_tuple(!instaswitch, speed) < std::make_tuple(!other.instaswitch, other.speed);
  }
};
// Doubles as choices available to player and player's choice
struct Choice {
  bool mega = false;
  std::array<bool, 4> move = {};
  std::array<bool, 6> swap = {};
  Choice() {}
  // Mega, move, swap
  Choice(bool meg, int mo, int sw) : mega(meg) {
    if (mo >= 0)
      move[mo] = true;
    if (sw >= 0)
      swap[sw] = true;
  }
};
enum class SwitchResult {
  FALSE,
  TRUE,
  PURSUIT_FAINT,
};
/*
What to call to initialize a Team:
- add_pokemon()
*/
struct Team {
  bool set = false;
  bool lock = false;
  std::array<Pokemon, 6> pkmn = {};
  int activeInd = -1;
  int pokemonLeft = 0;
  // Choices allowed
  bool instaSwitch = false;
  // Non-trivial mid-turn denotes actions left (e.g. fainted or U-Turn)
  Choice choicesAvailable;
  // Volatiles
  int tailwind = -1;    // Number of turns left of Tailwind (doubles speed)
  int auroraVeil = -1;  // Number of turns left of Aurora Veil (reduce both dmg)
  int lightScreen = -1; // Number of turns left of Light Screen (reduce special dmg)
  int reflect = -1;     // Number of turns left of Reflect (reduce physical dmg)
  int safeguard = -1;   // Number of turns left of Safeguard (protect from status)
  int luckyChant = -1;  // Number of turns left of Lucky Chant (block crit)
  // More volatiles
  bool faintedLastTurn = false; // For Retaliate
  bool faintedThisTurn = false; // For faintedLastTurn -> Retaliate

  bool add_pokemon(int slot, Pokemon pokemon, int lvl = 0);
  void lockTeam();
};
/*
What to call to initialize a BattleState:
- The contructor BattleState()
- startBattle()
- set_team(side, team) for each side
*/
class BattleState {
public:
  std::array<Team, 2> teams = {}; // Player is on side 0
  bool verbose = false;
  int turnNum = 0;
  bool midTurn = false;
  ActionKind currentAction = ActionKind::NONE;
  int lastFaintSide = -1;
  // Weather/terrain, etc.
  Weather weather = Weather::NO_WEATHER;
  int weatherTurns = 0;
  Terrain terrain = Terrain::NO_TERRAIN;
  int terrainTurns = 0;
  int gravity = 0;   // Number of turns left of Gravity, 0 for effect not applied.
  int trickRoom = 0; // Number of turns left of TrickRoom, 0 for effect not applied.
  // "Environmental"
  bool queueWillAct = false; // During move: whether more moves (switches N/A) remaining in queue
  int echoedVoiceMultiplier = 1; // Max of 5, only one increment per turn
  // DEBUG: SET MANUALLY
  DMGCalcOptions defaultDmgOptions = {};
  int secondaryDenom = 100;
  bool guaranteeHit = false; // Skip accuracy check on main hit
  // END DEBUG
  BattleState() {}
  BattleState(DMGCalcOptions ddo) : defaultDmgOptions(ddo) {}
  bool set_team(int side, Team &team);
  void set_switch_options(int side);
  void set_move_options(int side);
  void set_choices();
  bool startBattle();
  BattleState runTurnPy();
  void fillOpponentChoice(int side); // OPPONENT AI
  void runMove(MoveSlot &moveSlot, Pokemon &user, Pokemon &target);
  DamageResultState moveHit(Pokemon &target, Pokemon &user, MoveInstance &);
  int checkWin();
  DamageResultState getDamage(Pokemon &source, Pokemon &target, MoveInstance &moveInst,
                              DMGCalcOptions options); // Exported for test_init
  Pokemon &getActivePokemon(int side) { return teams[side].pkmn[teams[side].activeInd]; }
  int getActiveHP(int side) { return getActivePokemon(side).current_hp; }
  bool isInstaSwitch() { return teams[0].instaSwitch || teams[1].instaSwitch; }

private:
  // bool use_verbose_output = false;
  // std::vector<MoveInstance> verbose_outputs;
  bool battle_started = false;
  bool battle_ended = false;
  int winner = -1;
  std::queue<Pokemon *> faintQueue; // TODO: move to outer manager?
  bool isSunny() { return weather == Weather::SUNNY_DAY || weather == Weather::DESOLATE_LAND; }
  bool isRainy() { return weather == Weather::RAIN_DANCE || weather == Weather::PRIMORDIAL_SEA; }
  SwitchResult switchIn(int side, int switchToInd, bool isDrag);
  MoveActionPriority getMoveActionFromChoice(int side);
  SwitchAction getSwitchActionFromChoice(int side);
  void applyAtEndOfTurn();
  bool runTurn();

  /* START Event callbacks */

  void applyOnDisableMove(Pokemon &);
  void applyOnTrapPokemon(Pokemon &);
  void applyOnBeforeSwitchOut(Pokemon &);
  void applyOnSwitchOut(Pokemon &);
  void applyOnBeforeSwitchIn(Pokemon &);
  void applyOnWeatherChange(Pokemon &);
  void applyOnSwitchIn(Pokemon &);
  int applyOnBasePower(int basePower, const Pokemon &source, const Pokemon &target, const Move &);
  int applyBasePowerCallback(int basePower, Pokemon const &source, Pokemon const &target,
                             MoveInstance const &);
  bool applyOnCriticalHit(bool crit, Pokemon const &target, Pokemon const &source,
                          MoveInstance const &);
  int applyOnModifyAtk(int attack, Pokemon const &attacker, Pokemon const &defender, Move const &);
  int applyOnModifyDef(int defense, Pokemon const &defender);
  int applyOnModifySpA(int attack, Pokemon const &attacker, Pokemon const &defender, Move const &);
  int applyOnModifySpD(int defense, Pokemon const &defender);
  void applyOnStart(Pokemon &pokemon);
  void applyOnFlinch(Pokemon &pokemon);
  bool applyOnBeforeMoveST(Pokemon &pokemon);
  bool applyOnBeforeMove(Pokemon &pokemon, MoveInstance &);
  bool applyOnLockMove(Pokemon &user);
  void applyOnAfterMove(Pokemon &target, Pokemon &user, MoveInstance &);
  bool applyOnChargeMove(Pokemon &user);
  bool applyOnTryMove(Pokemon &target, Pokemon &user, MoveInstance &);
  int applyOnDamage(int damage, Pokemon &target, Pokemon &source, EffectKind, MoveId effectMove);
  void applyOnMoveFail(Pokemon &target, Pokemon &user, MoveInstance &);
  bool applyOnTry(Pokemon &user, Pokemon &target, MoveInstance &);
  bool applyOnStallMove(Pokemon &user);
  bool applyOnPrepareHit(Pokemon &user, Pokemon &target, MoveInstance &);
  bool applyOnInvulnerability(Pokemon &target, MoveInstance &);
  int applyOnModifyAccuracy(Pokemon const &target, Pokemon const &pokemon, MoveInstance &, int acc);
  bool applyOnAccuracy(Pokemon const &target, Pokemon const &pokemon, MoveInstance &);
  void applyOnAfterMoveSecondary(Pokemon &target, Pokemon &user, MoveInstance &);
  void applyOnEmergencyExit(int prevHP, Pokemon &pokemon);
  void applyOnAfterUseItem(Pokemon &target);
  bool applyOnTryHitStep(Pokemon &target, Pokemon &user, MoveInstance &);
  bool applyOnModifySecondaries(Pokemon &target, MoveInstance &, Secondary);
  bool applyOnDragOut(Pokemon &target);
  void applyOnDamagingHit(int damage, Pokemon &target, Pokemon &user, MoveInstance &);
  bool applyOnHitMain(Pokemon &target, Pokemon &user, MoveInstance &);
  void applyOnAfterHit(Pokemon &target, Pokemon &user, MoveInstance &);
  void applyOnUpdate(Pokemon &target);
  void applyOnResidual(Pokemon &target);
  void applyOnEach(EachEventKind);
  void applyOnEnd(Pokemon &);
  bool applyOnRestart(VolatileId, Pokemon &target, Pokemon &source);
  bool applyOnTryAddVolatile(VolatileId, Pokemon &target, Pokemon &source);
  bool applyOnStartVolatile(VolatileId, Pokemon &target, Pokemon &source, MoveId);
  bool applyOnSetStatus(Status, Pokemon &target, Pokemon &source, EffectKind, MoveInstance &);

  /* END Event Callbacks */

  bool canSwitch(int side);
  int damageCallback(Pokemon &source, Pokemon &target, MoveInstance &);
  void updateSpeed();
  void endTurn();
  std::optional<bool> applyAtEndOfAction(ActionKind);
  int calcRecoilDamage(int moveDmg, MoveInstance &moveInst, int userMaxHP);
  bool runSpecialImmunity(Pokemon &, EffectKind, bool isPrankster);
  bool runTypeImmunity(Pokemon &, Type);
  bool runStatusImmunity(Pokemon &, Status);
  bool isGrounded(Pokemon &, bool negateImmunity);
  bool setStatus(Status, Pokemon &target, Pokemon &source, EffectKind, MoveInstance &);
  bool addVolatile(VolatileId, Pokemon &target, Pokemon &source, MoveId);
  std::pair<int, int> getDrain(MoveId);
  int applyDamage(int damage, Pokemon &target);
  int directDamage(int damage, Pokemon &target, Pokemon &source, EffectKind);
  DamageResultState spreadDamage(DamageResultState, Pokemon &target, Pokemon &source, EffectKind,
                                 MoveId effectMove);
  void runMoveEffects(bool keepTargeting, Pokemon &target, Pokemon &source, MoveInstance &);
  bool useMoveInner(Pokemon &opp, Pokemon &pokemon, MoveInstance &);
  bool useMove(Pokemon &opp, Pokemon &pokemon, MoveInstance &);
  bool checkHit(Pokemon &user, Pokemon &target, MoveInstance &, bool changeState);
  bool trySpreadMoveHit(Pokemon &target, Pokemon &pokemon, MoveInstance &);
  bool applySecondaryEffect(Pokemon &target, Pokemon &source, SecondaryEffect, MoveInstance &);
  void faint(Pokemon &pkmn);
  void faintMessages();
};

void sortMoveActions(std::vector<MoveActionPriority> &moveActions);
void sortSwitchActions(std::vector<SwitchAction> &switchActions);
} // namespace pkmn