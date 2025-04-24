#include "battle_state.hpp"
#include "utils.hpp"

#include <optional>
namespace pkmn {
// Returns whether or not a team that had been set was overwritten.
bool BattleState::set_team(int side, Team &team) {
  if (teams[side].lock) {
    std::cerr << "[BattleState::set_team] Tried changing locked team." << std::endl;
  }
  bool overwritten = teams[side].set;
  team.set = true;
  teams[side] = team; // copies
  return overwritten;
}
// Returns if there an unfainted ally for the active Pokemon to switch to.
bool BattleState::canSwitch(int side) {
  return teams[side].pokemonLeft > 1 || !getActivePokemon(side).current_hp;
}
// Could be called while non-switching Pokemon haven't moved yet.
// TODO: reference Side.chooseSwitch() for correct logic
// TODO: check that logic for locking/trapping in switchFlag is correct.
void BattleState::set_switch_options(int side) {
  Team &team = teams[side];
  Pokemon &activePkmn = team.pkmn[team.activeInd];
  bool activePkmnCanSwitch = canSwitch(side) && (!midTurn || activePkmn.switchFlag);
  Choice &choice = team.choicesAvailable;
  choice = {}; // Reset all choices that can be made
  for (int j = 0; j < 6; j++) {
    Pokemon &candidate = team.pkmn[j];
    // Cannot switch into current active Pokemon
    if (activePkmnCanSwitch && team.activeInd != j && candidate.species != Species::NONE &&
        !candidate.fainted) {
      choice.swap[j] = true;
    }
  }
}
// Only called at end of turn, in set_choices().
// TODO: reference Side.chooseMove() for correct logic
// TODO: locking, Struggle, etc.
void BattleState::set_move_options(int side) {
  Pokemon &activePkmn = getActivePokemon(side);
  Choice &choice = teams[side].choicesAvailable;
  for (int j = 0; j < 4; j++) {
    choice.move[j] = false;
    const MoveSlot move = activePkmn.moves[j];
    if (move.id != MoveId::NONE && !move.disabled && move.pp > 0) {
      choice.move[j] = true;
    }
  }
}
// Called at the end of a turn to completely reset choices
// TODO: set_mega_options()
void BattleState::set_choices() {
  for (int side = 0; side < 2; side++) {
    teams[side].choicesAvailable = {};
    set_switch_options(side); // Removes other options
    // set_mega_options(); // Does not remove potential switch options
    set_move_options(side); // Does not remove potential switch options
  }
}
// TODO: counter/mirrorcoat, metalburst
int BattleState::damageCallback(Pokemon &source, Pokemon &target, MoveInstance &moveInst) {
  switch (moveInst.id) {
  case MoveId::COUNTER:
  case MoveId::MIRRORCOAT: {
    break;
  }
  case MoveId::ENDEAVOR: {
    return std::max(0, target.current_hp - source.current_hp);
  }
  case MoveId::FINALGAMBIT: {
    int dmg = source.current_hp;
    faint(source);
    return dmg;
  }
  case MoveId::METALBURST: {
    // const lastDamagedBy = pokemon.getLastDamagedBy(true);
    // if (lastDamagedBy !== undefined) {
    // 	return (lastDamagedBy.damage * 1.5) || 1;
    // }
    // return 0;
    break;
  }
  case MoveId::NATURESMADNESS:
  case MoveId::RUINATION:
  case MoveId::SUPERFANG: {
    return std::max(1, target.stats.hp / 2);
  }
  case MoveId::PSYWAVE: {
    return (math::random(50, 151) * source.lvl) / 100;
  }
  // case MoveId::COMEUPPANCE: // Gen9
  default:
    return -1;
  }
}
// Assumes move.power != std::nullopt
// Called by getSpreadDamage() and Substitute::onTryPrimaryHit() logic
// TODO:
// - modifyDamage
//   - Weather
//   - Guts
//   - onModifyDamage
// Do later:
// - Substitute immunity check
// - overridePokemon
// - modifyDamage
//   - Handle ??? in STAB
DamageResultState BattleState::getDamage(Pokemon &source, Pokemon &target, MoveInstance &moveInst,
                                         DMGCalcOptions options) {
  DamageResultState ret;
  const Move &move = moveInst.moveData;
  // Do later: Immunity check for call from Substitute
  if (moveInst.isOHKO()) {
    return {target.stats.hp};
  }
  int dmgFromCallback = damageCallback(source, target, moveInst);
  if (dmgFromCallback >= 0) {
    return {dmgFromCallback};
  }
  if (move.id == MoveId::NIGHTSHADE || move.id == MoveId::SEISMICTOSS) {
    return {source.lvl};
    ret.set_numeric(source.lvl);
    return ret;
  } else if (move.id == MoveId::DRAGONRAGE) {
    ret.set_numeric(40);
    return ret;
  } else if (move.id == MoveId::SONICBOOM) {
    ret.set_numeric(20);
    return ret;
  }

  int basePower = move.basePower;
  basePower = applyBasePowerCallback(basePower, source, target, moveInst);
  basePower = std::max(1, basePower);
  // Crit RNG
  bool moveCrit = getCrit(target, source, moveInst.id, options);
  if (moveCrit) {
    moveCrit = applyOnCriticalHit(moveCrit, target, source, moveInst);
  }
  basePower = applyOnBasePower(basePower, source, target, move);
  int level = source.lvl;
  // Do later: overridePokemon -- just Offensive, from Foul Play
  Pokemon &attacker = move.id == MoveId::FOULPLAY ? target : source;
  Pokemon &defender = target;
  EffectiveStatVals stats = getEffectiveStats(attacker, defender, move);
  if (moveCrit) { // Boosts ignore negative attacking and positive defending stats
    stats.attBoost = std::max(0, stats.attBoost);
    stats.defBoost = std::min(0, stats.defBoost);
  }
  // calculateStat
  int attack = attacker.calculateStat(stats.attStatName, stats.attBoost, source);
  int defense = defender.calculateStat(stats.defStatName, stats.defBoost, target);
  // onModifyAtk() and onModifyDef()
  // Note these callbacks aren't redirected by e.g. FoulPlay
  // Offensive signature: (stat, attacker, defender, move)
  // Defensive signature: (stat, defender)
  if (move.category == MoveCategory::PHYSICAL) {
    attack = applyOnModifyAtk(attack, source, target, move);
    defense = applyOnModifyDef(defense, target);
  } else {
    attack = applyOnModifySpA(attack, source, target, move);
    defense = applyOnModifySpD(defense, target);
  }
  int baseDamage = (2 * level / 5 + 2) * basePower * attack / defense / 50;
  // From here: modifyDamage
  baseDamage += 2; // Intriguing
  // Spread. For single battles: Parental Bond
  if (moveInst.parentalBond && moveInst.currentHitNum > 1) {
    baseDamage /= 4;
  }
  // TODO: Weather
  // Crit multiplier (multiplier: 1.5)
  if (moveCrit) {
    baseDamage = applyModifier(baseDamage, 3, 2);
  }
  // Randomization (16 rolls)
  baseDamage = generateAndApplyDmgRoll(baseDamage, options);
  // STAB (TODO: check database for ??? type)
  baseDamage = checkAndApplySTAB(baseDamage, attacker, move);
  // Type effectiveness
  std::optional<int> typeMod = getTypeMod(defender, move.type);
  if (typeMod == std::nullopt) {
    ret.set_numeric(0);
    return ret;
  } else {
    baseDamage = applyTypeEffectiveness(baseDamage, *typeMod);
  }
  // TODO: Guts modifier
  // TODO: onModifyDamage() -- items, Flash Fire. Ignore phases (old gen)
  if (baseDamage == 0) { // Min damage check
    ret.set_numeric(1);
  } else {
    ret.set_numeric(baseDamage % (1 << 16));
  }
  return ret;
}
// Given an impetus to switch (decision or drag), does the actual action of
// switching in the new Pokemon for the old Pokemon as active.
// Returns a SwitchResult enum for directness
// TODO: Figure out isDrag flag and switchIn() vs. runSwitch()
// Do later: BatonPass flag/copyVolatiles, illusion flag, effectOrder
SwitchResult BattleState::switchIn(int side, int switchToInd, bool isDrag) {
  Team &team = teams[side];
  Pokemon &pokemon = team.pkmn[switchToInd]; // Use reference
  if (pokemon.isActive)
    return SwitchResult::FALSE;
  if (teams[side].activeInd >= 0) { // Otherwise is start of battle
    Pokemon &oldActive = team.pkmn[team.activeInd];
    if (oldActive.current_hp > 0) { // unfaintedActive
      oldActive.beingCalledBack = true;
      // Do later: Baton Pass flag set
      // let switchCopyFlag: 'copyvolatile' | 'shedtail' | boolean = false;
      // if (sourceEffect && typeof (sourceEffect as Move).selfSwitch === 'string') {
      // 	switchCopyFlag = (sourceEffect as Move).selfSwitch!;
      // }
      if (!oldActive.skipBeforeSwitchOutEventFlag && !isDrag) {
        applyOnBeforeSwitchOut(oldActive);
        applyOnEach(EachEventKind::UPDATE);
      }
      oldActive.skipBeforeSwitchOutEventFlag = false;
      applyOnSwitchOut(oldActive); // Just Regenerator
      if (!oldActive.current_hp)
        return SwitchResult::PURSUIT_FAINT;
      // Do later: oldActive.illusion = false;
      applyOnEnd(oldActive);
      // Do later: Baton Pass copyVolatile
      oldActive.clearVolatile(true);
    }
    oldActive.isActive = false;
    oldActive.isStarted = false;
    oldActive.usedItemThisTurn = false;
    oldActive.statsRaisedThisTurn = false;
    oldActive.statsLoweredThisTurn = false;
    if (oldActive.fainted)
      oldActive.status = Status::NO_STATUS;
    // Swap positions for potential DragIn shenanigans
    std::swap(pokemon.position, oldActive.position);
  }
  pokemon.isActive = true;
  team.activeInd = switchToInd;
  pokemon.activeTurns = 0;
  pokemon.activeMoveActions = 0;
  for (int slot = 0; slot < 4; slot++) {
    pokemon.moves[slot].used = false;
  }
  // pokemon.abilityState.effectOrder = effectOrder++;
  // pokemon.itemState.effectOrder = effectOrder++;
  applyOnBeforeSwitchIn(pokemon); // Just Illusion
  // previouslySwitchedIn is only for Gen9 Libero/Protean

  // runSwitch just runs onSwitchIn for switched-in Pokemon in speed order
  // // So the order is...
  // // Calling runAction('switch') checks switchIn()==pursuitfaint
  // // But switchIn() only directly does runSwitch() if dragging in
  // // runSwitch() just collects all available switch choices and executes them in speed order
  // // What can happen in between? How can I flag so runTurn() knows whether to call runSwitch()?
  // if(isDrag) {
  //   runSwitch(pokemon);
  // } else {
  //   // Add runSwitch to action queue
  // }
  applyOnSwitchIn(pokemon);
  if (pokemon.current_hp) {
    pokemon.isStarted = true;
  }
  return SwitchResult::TRUE;
}
// Should only be run after this's fields are validly populated
// Should not be run if battle_started==true
// Returns false if there's an error of these sorts.
// Do later: figure out why onStart() is run for non-active Pokemon
bool BattleState::startBattle() {
  // start() calls turnLoop() which calls runAction('start').
  // Only endTurn() should be run from applyAtEndOfAction?
  if (battle_started) {
    return false;
  }
  battle_started = true;
  midTurn = true;
  for (int side = 0; side < 2; side++) {
    // We'll just check the lead Pokemon's rough validity
    Pokemon &leadPokemon = teams[side].pkmn[0];
    if (leadPokemon.species == Species::NONE || leadPokemon.moves[0].id == MoveId::NONE) {
      return false;
    }
    teams[side].pokemonLeft = 0;
    // Mark the side of each non-empty Pokemon slot
    bool lastPokemon = false;
    for (int i = 0; i < 6; i++) {
      Pokemon &pkmn = teams[side].pkmn[i];
      if (pkmn.species != Species::NONE) {
        if (lastPokemon) {
          std::cerr << "Did not find Pokemon to be in consecutive positions!" << std::endl;
        }
        pkmn.side = side;
        pkmn.position = i;
        pkmn.effectiveSpeed = pkmn.stats.spd;
        teams[side].pokemonLeft++;
      } else {
        lastPokemon = true;
      }
    }
    teams[side].lockTeam();
  }
  // Switch in the lead Pokemon
  for (int side = 0; side < 2; side++) {
    switchIn(side, 0, false);
  }
  // Runs onStart() for each *pokemon*, not just active?
  for (int side = 0; side < 2; side++) {
    for (int i = 0; i < 6; i++) {
      if (teams[side].pkmn[i].species != Species::NONE) {
        applyOnStart(teams[side].pkmn[i]);
      }
    }
  }
  endTurn();
  midTurn = false;
  set_choices();
  return true;
}
// Break ties by RNG
void sortMoveActions(std::vector<MoveActionPriority> &moveActions) {
  if (moveActions.size() >= 2) { // Should be equal to 2, then
    if (moveActions[1] < moveActions[0] ||
        (!(moveActions[0] < moveActions[1]) && math::randomChance(1, 2))) {
      std::swap(moveActions[0], moveActions[1]);
    }
  }
}
// Break ties by RNG
void sortSwitchActions(std::vector<SwitchAction> &switchActions) {
  if (switchActions.size() >= 2) { // Should be equal to 2, then
    if (switchActions[1] < switchActions[0] ||
        (!(switchActions[0] < switchActions[1]) && math::randomChance(1, 2))) {
      std::swap(switchActions[0], switchActions[1]);
    }
  }
}
// Validate move selection and get struct holding the move's priority, user's speed, and any
// fractional priority
// TODO: figure out fractionalPriority
// Do later: Triage, QuickGuard
MoveActionPriority BattleState::getMoveActionFromChoice(int side) {
  MoveActionPriority ret;
  Team &team = teams[side];
  Pokemon &pokemon = team.pkmn[team.activeInd];
  ret.moveInd = -1;
  for (int i = 0; i < 4; i++) {
    if (team.choicesAvailable.move[i]) {
      if (ret.moveInd >= 0) {
        std::cerr << "[BattleState::getMoveActionFromChoice] multiple moves selected by team "
                  << side << std::endl;
      }
      ret.moveInd = i;
    }
  }
  if (ret.moveInd >= 0) { // A move was selected
    MoveId id = pokemon.moves[ret.moveInd].id;
    Move &move = moveDict.dict[id];
    ret.side = side;
    ret.speed = pokemon.effectiveSpeed; // Uses boosts/modifiers/TrickRoom
    /* TODO: Fractional priority:
    // abilities Mycelium Might, Quick Draw, and Stall
    // items Custap Berry, Full Incense, Lagging Tail, and Quick Claw
    // ret.fractionalPriority = ...;
    */
    ret.priority = moveDict.dict[id].priority;
    // onModifyPriority():
    if (pokemon.ability == Ability::GALE_WINGS && move.type == Type::FLYING &&
        pokemon.current_hp == pokemon.stats.hp) {
      ret.priority++;
    }
    if (pokemon.ability == Ability::PRANKSTER && move.category == MoveCategory::STATUS) {
      ret.priority++;
    }
    // if(pokemon.ability == Ability::TRIAGE && move.isHeal().first > 0) {
    //   ret.priority++;
    // }
    // Maybe need shenanigans for Quick Guard? eh
  }
  return ret;
}
// Validate selection and get struct holding the switching Pokemon's speed
SwitchAction BattleState::getSwitchActionFromChoice(int side) {
  SwitchAction ret;
  Team &team = teams[side];
  Pokemon &pokemon = team.pkmn[team.activeInd];
  ret.newPokeInd = -1;
  for (int i = 0; i < 6; i++) {
    if (team.choicesAvailable.swap[i]) {
      if (ret.newPokeInd >= 0) {
        std::cerr << "[BattleState::getSwitchActionFromChoice] multiple swaps selected by team "
                  << side << std::endl;
      }
      ret.newPokeInd = i;
    }
  }
  if (ret.newPokeInd >= 0) { // A swap was selected
    if (ret.newPokeInd == team.activeInd) {
      std::cerr
          << "[BattleState::getSwitchActionFromChoice] swap to current active Pokemon by team "
          << side << std::endl;
    }
    ret.side = side;
    ret.speed = pokemon.effectiveSpeed;
    pokemon.switchFlag = false;
  }
  return ret;
}
// Main call exposed by API.
// Note that it manipulates and returns a *copy* of itself.
// Do later: refactor so it wraps a stateful Battle around a BattleState
BattleState BattleState::runTurnPy() {
  BattleState ret{*this};
  ret.runTurn();
  return ret;
}
// Update speed of each active Pokemon.
// Does shenanigans for trick room.
void BattleState::updateSpeed() {
  for (int side = 0; side < 2; side++) {
    Pokemon &activePkmn = teams[side].pkmn[teams[side].activeInd];
    int spd = activePkmn.getStat(ModifierId::SPEED, true, true);
    activePkmn.effectiveSpeed = trickRoom ? 10000 - spd : spd;
  }
}
// Do later: Disable/Locking, Trapping, figure out what staleness is
void BattleState::endTurn() {
  turnNum++;
  // lastSuccessfulMoveThisTurn = MoveId::NONE;
  for (int side = 0; side < 2; side++) {
    Pokemon &pkmn = getActivePokemon(side);
    pkmn.newlySwitched = false;
    pkmn.moveLastTurnFailed = pkmn.moveThisTurnFailed;
    pkmn.moveThisTurnFailed = false;
    pkmn.usedItemThisTurn = false;
    pkmn.statsRaisedThisTurn = false;
    pkmn.statsLoweredThisTurn = false;
    pkmn.wasHurtThisTurn = false;
    // // Do later: DISABLE MOVE CHOICES: DISABLE/LOCKING
    // pkmn.maybeDisabled = false;
    // pkmn.maybeLocked = false;
    for (int i = 0; i < 4; i++) {
      pkmn.moves[i].disabled = false;
    }
    applyOnDisableMove(pkmn);
    // Note: cantusetwice flag is only for Gen9 moves
    // Do later: update attackedBy
    // TRAPPING
    pkmn.trapped = false;
    applyOnTrapPokemon(pkmn);
    // applyOnMaybeTrapPokemon(pkmn); // No such callback?
    if (pkmn.fainted)
      continue;
    // // Not sure what staleness means
    // const staleness = pokemon.volatileStaleness || pokemon.staleness;
    // if (staleness) sideStaleness = sideStaleness === 'external' ? sideStaleness : staleness;
    // stalenessBySide.push(sideStaleness);
    teams[side].faintedLastTurn = teams[side].faintedThisTurn;
    teams[side].faintedThisTurn = false;
    pkmn.activeTurns++;
    pkmn.decrVolDurations();
  }
}
// Applies phaze, faintMessage, and instaSwitch flag setup.
// If mid-turn switch is called for, sets instaSwitch flags and returns true
// since the battle hasn't ended yet.
// TODO: phazing, EmergencyExit on residual
// Do later: figure out what clearActiveMove() is for
std::optional<bool> BattleState::applyAtEndOfAction(ActionKind action) {
  // TODO: phaze
  // clearActiveMove(failed=false);
  faintMessages();
  if (battle_ended)
    return {false};
  // Fainted Pokemon only get to switch right before the turn ends.
  if (action == ActionKind::RESIDUAL) {
    for (int side = 0; side < 2; side++) {
      Pokemon &activePkmn = teams[side].pkmn[teams[side].activeInd];
      if (activePkmn.fainted) {
        // std::cout << "[BattleState::applyAtEndOfAction] Set faint switch flag for side " << side
        // << std::endl;
        activePkmn.switchFlag = true;
        teams[side].instaSwitch = true;
      }
    }
  }
  applyOnEach(EachEventKind::UPDATE);
  // TODO: run applyOnEmergencyExit for any residual-affected Pokemon
  // if(action == EndOfActionKind::RESIDUAL) {
  //   applyOnEmergencyExit(originalHP, pokemon);
  // }

  std::array<bool, 2> switches{};
  for (int side = 0; side < 2; side++) {
    Pokemon &activePkmn = getActivePokemon(side);
    if (activePkmn.switchFlag) {
      switches[side] = true;
      if (canSwitch(side)) {
        if (activePkmn.current_hp && !activePkmn.skipBeforeSwitchOutEventFlag) {
          applyOnBeforeSwitchOut(activePkmn);
          activePkmn.skipBeforeSwitchOutEventFlag = true;
          faintMessages(); // Pokemon may have fainted in BeforeSwitchOut
          if (battle_ended)
            return {false};
          if (activePkmn.fainted) {
            switches[side] = getActivePokemon(side).switchFlag;
          }
        }
      } else {
        activePkmn.switchFlag = false;
      }
    }
    if (switches[side]) {
      set_switch_options(side);
    }
  }
  // Post-faint switches: request simultaneously
  if (switches[0] || switches[1]) {
    // std::cout << "Requesting instaswitch\n";
    return {true};
  }
  // Continue with further actions
  return std::nullopt;
}
// Returns false if either the battle is unstarted or ended.
// Do later:
// - megaEvo action
// - beforeTurnMove action
// - priorityChargeMove action
bool BattleState::runTurn() {
  if (!battle_started || battle_ended)
    return false;
  midTurn = true;
  if (currentAction == ActionKind::NONE) {
    updateSpeed();
  }
  // Do later: megaEvos
  if (currentAction <= ActionKind::MEGA) {
    currentAction = ActionKind::MEGA;
  }
  // Do later: 'beforeTurnMove' action (if applicable, i.e. Counter, Mirror Coat, Pursuit)
  if (currentAction <= ActionKind::SWITCH || isInstaSwitch()) {
    currentAction = ActionKind::SWITCH;
    // Create SwitchActions
    std::vector<SwitchAction> switchActions;
    for (int side = 0; side < 2; side++) {
      SwitchAction switchAct = getSwitchActionFromChoice(side);
      if (switchAct.newPokeInd >= 0) {
        switchActions.push_back(switchAct);
        if(teams[side].instaSwitch) {
          switchActions.back().instaswitch = true;
        }
      }
    }
    // NB: 'switch' sets up callbacks (e.g. beforeTurnMove) and calls 'runSwitch'
    // 'runSwitch' executes available switch choices in order
    sortSwitchActions(switchActions);
    for (size_t i = 0; i < switchActions.size(); i++) {
      int side = switchActions[i].side;
      // Reset choice
      if(verbose) {
        // std::cout << "Resetting choices for side " << side << std::endl;
      }
      teams[side].choicesAvailable = Choice{};
      teams[side].instaSwitch = false;
      switchIn(side, switchActions[i].newPokeInd, false);
      auto res = applyAtEndOfAction(ActionKind::SWITCH);
      if (res != std::nullopt) {
        std::cout << "[BattleState::runTurn] return at switchAction\n";
        return *res;
      }
    }
  }
  // Do later: 'priorityChargeMove' action
  if (currentAction <= ActionKind::MOVE) {
    currentAction = ActionKind::MOVE;
    // Create MoveActions
    std::vector<MoveActionPriority> moveActions;
    for (int side = 0; side < 2; side++) {
      MoveActionPriority moveAP = getMoveActionFromChoice(side);
      if (moveAP.moveInd >= 0) {
        moveActions.push_back(moveAP);
      }
    }
    sortMoveActions(moveActions);
    for (size_t i = 0; i < moveActions.size(); i++) {
      queueWillAct = i + 1 < moveActions.size();
      auto moveAction = moveActions[i];
      Team &team = teams[moveAction.side];
      Team &otherTeam = teams[1 - moveAction.side];
      Pokemon &pokemon = team.pkmn[team.activeInd];
      MoveSlot &moveSlot = pokemon.moves[moveAction.moveInd];
      // std::cout << "Current mover is from side: " << moveAction.side << std::endl;
      // std::cout << "Priority: " << moveAction.priority << std::endl;
      // Reset choice
      if(verbose) {
        // std::cout << "Resetting choices for side " << moveAction.side << std::endl;
      }
      team.choicesAvailable = Choice{};
      // Cannot use move if switched out e.g. by Whirlwind or EjectButton
      if (pokemon.isActive) {
        runMove(moveSlot, pokemon, otherTeam.pkmn[otherTeam.activeInd]);
        auto res = applyAtEndOfAction(ActionKind::MOVE);
        if (res != std::nullopt) {
          // std::cout << "[BattleState::runTurn] return at moveAction\n";
          return *res;
        }
      }
    }
  }
  if (currentAction <= ActionKind::RESIDUAL) {
    currentAction = ActionKind::RESIDUAL;
    // clearActiveMove(failed=true);
    updateSpeed();
    applyOnEach(EachEventKind::RESIDUAL);
    auto res = applyAtEndOfAction(ActionKind::RESIDUAL);
    if (res != std::nullopt) {
      // std::cout << "[BattleState::runTurn] return at residualAction\n";
      return *res;
    }
  }
  endTurn();
  midTurn = false;
  if(verbose) {
    // std::cout << "Resetting choices for both sides...\n";
  }
  set_choices();
  currentAction = ActionKind::NONE;
  return true;
}
// Update winner if found
// Returns winning side, or -1 if battle has not ended.
int BattleState::checkWin() {
  if (battle_ended)
    return winner;
  // If neither side has pokemon left, the last-fainted side wins
  if (!teams[0].pokemonLeft && !teams[1].pokemonLeft) {
    battle_ended = true;
    if (!faintQueue.empty()) {
      winner = faintQueue.back()->side;
    } else if (lastFaintSide >= 0) {
      winner = lastFaintSide;
    } else {
      std::cerr << "[BattleState::checkWin] Don't know how to resolve winner!" << std::endl;
      return -1; // Error b/c invalid return value
    }
  }
  // Otherwise whichever side has pokemon left wins
  for (int side = 0; side < 2; side++) {
    if (!teams[side].pokemonLeft) {
      battle_ended = true;
      winner = 1 - side;
    }
  }
  return winner; // Game might not have ended yet
}
// Given that a move hits, apply chain of events.
// Do later: Encore, onMoveAborted(), boforeMoveCallback, lastMoveTargetLoc, moveThisTurn,
// lastSuccessfulMoveThisTurn, activeMove Skip: Dancer
void BattleState::runMove(MoveSlot &moveSlot, Pokemon &user, Pokemon &target) {
  user.activeMoveActions++;
  // instantiate the moveInst from the moveSlot
  MoveInstance moveInst = MoveInstance(moveSlot.id);
  // Do later: potential Encore move replacement/retargeting (onOverrideAction) with Prankster boost
  // onBeforeMove():
  bool stompingTantrumFail = applyOnBeforeMoveST(user);
  bool willTryMove = !stompingTantrumFail && applyOnBeforeMove(user, moveInst);
  if (!willTryMove) {
    // TODO: onMoveAborted(user, target, move); // Remove volatiles that might've already been added
    // clearActiveMove(failed=true);
    user.moveThisTurnFailed = stompingTantrumFail;
    return;
  }
  // // beforeMoveCallback: Bide, FocusPunch
  // if(!beforeMoveCallback(target, user, moveInst)) {
  //   // Move failed
  //   // clearActiveMove(failed=true);
  //   user.moveThisTurnFailed = true;
  //   return;
  // }
  // Skip: pokemon.lastDamage, never read
  bool lockedMove = applyOnLockMove(user);
  if (!lockedMove) {
    if (!user.deductPP(moveSlot) && moveInst.id != MoveId::STRUGGLE) {
      // clearActiveMove(fail=true);
      user.moveThisTurnFailed = true;
      return;
    }
  } else {
    // sourceEffect = conditions.lockedmove;
  }
  user.lastMove = moveInst.id;
  // Do later:
  // user.lastMoveTargetLoc = ...
  // user.moveThisTurn = ...

  bool moveDidSomething = useMove(target, user, moveInst);
  // std::cout << "pokemon on side " << target.side << " now has HP: " << target.current_hp << std::endl;
  // Do later: update lastSuccessfulMoveThisTurn for FusionFlare/Bolt
  // lastSuccessfulMoveThisTurn = moveDidSomething ? moveInst.id : MoveId::NONE;
  // if (activeMoveInst) moveInst = activeMoveInst;
  applyOnAfterMove(target, user, moveInst);
  // don't do: Weird Dancer logic
  faintMessages();
  checkWin();
}
// false for immune, true for not-immune
// Checks Attract, Powder, weather (Sandstorm/Hail), prankster-boosted
bool BattleState::runSpecialImmunity(Pokemon &target, EffectKind effectKind, bool isPrankster) {
  if (isPrankster && target.has_type(Type::DARK)) {
    return false;
  }
  switch (effectKind) {
  case EffectKind::ATTRACT: {
    if (target.has_ability(Ability::OBLIVIOUS)) {
      return false;
    }
    break;
  }
  case EffectKind::POWDER: {
    if (target.has_type(Type::GRASS) || target.has_ability(Ability::OVERCOAT) ||
        target.has_item(Item::SAFETY_GOGGLES)) {
      return false;
    }
    break;
  }
  case EffectKind::SANDSTORM: {
    if (target.has_type(Type::GROUND) || target.has_type(Type::STEEL) ||
        target.has_type(Type::ROCK) || target.has_ability(Ability::OVERCOAT) ||
        target.has_ability(Ability::SAND_FORCE) || target.has_ability(Ability::SAND_RUSH) ||
        target.has_ability(Ability::SAND_VEIL) || target.has_item(Item::SAFETY_GOGGLES) ||
        target.underground || target.underwater) {
      return false;
    }
    break;
  }
  case EffectKind::HAIL: {
    if (target.has_type(Type::ICE) || target.has_ability(Ability::ICE_BODY) ||
        target.has_ability(Ability::OVERCOAT) || target.has_ability(Ability::SNOW_CLOAK) ||
        target.has_item(Item::SAFETY_GOGGLES) || target.underground || target.underwater) {
      return false;
    }
    break;
  }
  default:
    break;
  }
  return true;
}
// false for immune, true for not-immune
bool BattleState::runTypeImmunity(Pokemon &target, Type typ) {
  if (typ == Type::NO_TYPE)
    return true;
  if (target.fainted)
    return false;
  // applyOnNegateImmunity(target, typ):
  bool negateImmunity = false;
  if (target.has_item(Item::RING_TARGET))
    negateImmunity = true;
  if (target.foresight && target.has_type(Type::GHOST)) {
    if (typ == Type::NORMAL || typ == Type::FIGHTING) {
      negateImmunity = true;
    }
  }
  if (target.miracleeye && target.has_type(Type::DARK) && typ == Type::PSYCHIC) {
    negateImmunity = true;
  }
  // Final return
  if (typ == Type::GROUND) {
    return isGrounded(target, negateImmunity);
  }
  auto typeMod = getTypeMod(target, typ);
  return typeMod != std::nullopt;
}
// false for immune, true for not-immune
bool BattleState::runStatusImmunity(Pokemon &target, Status status) {
  if (target.fainted)
    return false;
  // No status was actually given
  if (status == Status::NO_STATUS) {
    return false;
  }

  // Natural type immunities
  if (target.has_type(Type::ELECTRIC) && status == Status::PARALYSIS) {
    return false;
  } else if (target.has_type(Type::FIRE) && status == Status::BURN) {
    return false;
  } else if (target.has_type(Type::ICE) && status == Status::FREEZE) {
    return false;
  } else if ((target.has_type(Type::POISON) || target.has_type(Type::STEEL)) &&
             (status == Status::POISON || status == Status::TOXIC)) {
    return false;
  }

  // "Artificial" immunities
  if (status == Status::FREEZE) {
    if (target.has_ability(Ability::MAGMA_ARMOR)) {
      return false;
    } else if (weather == Weather::DESOLATE_LAND || weather == Weather::SUNNY_DAY) {
      if (!target.has_item(Item::UTILITY_UMBRELLA)) {
        return false;
      }
    }
  }
  return true;
}
// Under consideration, in order:
// Gravity, Ingrain, Smackdown, IronBall, Flying-type, Levitate, MagnetRise,
// Telekinesis, AirBalloon.
// Do later: finish IronBall, BurnUp+Roost special case, ability suppressed
bool BattleState::isGrounded(Pokemon &pokemon, bool negateImmunity) {
  if (gravity)
    return true;
  if (pokemon.ingrained)
    return true;
  if (pokemon.smackeddown)
    return true;
  //  Do later: ignoringItem
  if (pokemon.has_item(Item::IRON_BALL))
    return true;
  // Do later: BurnUp+Roost special case
  if (!negateImmunity && pokemon.has_type(Type::FLYING) && !pokemon.roosted)
    return false;
  // Do later: ability suppressed
  if (pokemon.has_ability(Ability::LEVITATE)) {
    return false;
  }
  if (pokemon.magnetrise)
    return false;
  if (pokemon.telekinesised)
    return false;
  if (pokemon.has_item(Item::AIR_BALLOON))
    return false;
  return true;
}
// Tries applying a status to target, including with all initialization/checks/effects.
// Returns whether or not the status was successfully applied.
// TODO: finish Synchronize
// Do later: cure nightmare, formeChange Shaymin
bool BattleState::setStatus(Status newStatus, Pokemon &target, Pokemon &source,
                            EffectKind effectKind, MoveInstance &moveInst) {
  if (!target.current_hp)
    return false;
  if (newStatus == target.status)
    return false;
  // Only check immunity if not pierced by corrosion
  if (!(source.has_ability(Ability::CORROSION) &&
        (newStatus == Status::TOXIC || newStatus == Status::POISON)) &&
      !runStatusImmunity(target, newStatus)) {
    return false;
  }
  if (!applyOnSetStatus(newStatus, target, source, effectKind, moveInst))
    return false;
  target.status = newStatus;
  // applyOnStart() -- Status version:
  if (target.status == Status::SLEEP) {
    target.sleepTurns = math::random(2, 5);
    // Do later: cure nightmare
  } else if (target.status == Status::FREEZE) {
    // formeChange Shaymin-Sky back to Shaymin
  } else if (target.status == Status::TOXIC) {
    target.toxicStage = 0;
  }
  // applyOnAfterSetStatus():
  if (target.has_ability(Ability::SYNCHRONIZE)) {
    if(source != target && effectKind != EffectKind::TOXIC_SPIKES && newStatus != Status::SLEEP && newStatus != Status::FREEZE) {
      setStatus(newStatus, source, target, EffectKind::SYNCHRONIZE, moveInst);
    }
  }
  if (target.has_item(Item::LUM_BERRY)) {
    target.useItem(true, false);
  }
  return true;
}
// Tries applying a volatile condition to target, including with all initialization/checks/effects.
// Returns whether or not the volatile was successfully applied.
// TODO: Initialize volatile
// Do later: AnchorShot/Block, linkedStatus, effectOrder
bool BattleState::addVolatile(VolatileId vol, Pokemon &target, Pokemon &source, MoveId move) {
  // e.g. Gem can boost Explosion damage
  if (!target.current_hp && vol != VolatileId::GEM)
    return false;
  // Do later: AnchorShot and Block only trap as long as user stays in
  // if (linkedStatus && source && !source.hp) return false;
  if (target.has_volatile(vol)) {
    return applyOnRestart(vol, target, source);
  }
  // Attract
  if (vol == VolatileId::ATTRACT) {
    if (!runSpecialImmunity(target, EffectKind::ATTRACT, false)) {
      return false;
    }
  }
  if (!applyOnTryAddVolatile(vol, target, source)) {
    return false;
  }

  // Q: Why initialize volatile *before* calling onStart if you might cancel anyways?
  // For now, just check onStart first
  if (applyOnStartVolatile(vol, target, source, move)) {
    // TODO: initialize corresponding volatile, possibly with duration
    // Do later: effectOrder is to run volatile effects in the order in which they were applied
    switch (vol) {
    case VolatileId::FLINCH: {
      target.flinch = true;
      break;
    }
    case VolatileId::LEECHSEED: {
      target.leechSeed = true;
      break;
    }
    default:
      break;
    }
    return true;
  } else {
    return false;
  }
}
// Actually apply the damage (capped by current HP)
// Called by spreadDamage() and directDamage()
// PokemonShowdown Pokemon.damage()
// Returns the amount HP actually decreased by.
int BattleState::applyDamage(int damage, Pokemon &target) {
  if (target.current_hp <= 0 || damage <= 0)
    return 0;
  // std::cout << "[BattleState::applyDamage] " << damage << " dmg applied to side " << target.side
  // << std::endl;
  target.current_hp -= damage;
  if (target.current_hp <= 0) {
    damage += target.current_hp;
    target.current_hp = 0;
    faint(target);
  }
  return damage;
}
// We leave this unimplemented and directly call applyDamage
// There seems to basically just be Gen1 logic here, since rounding is implicit in C++
// and fainted should only be set on faintMessages(), which happens after faint() is called.
int BattleState::directDamage(int damage, Pokemon &target, Pokemon &source, EffectKind effectKind) {
  return 0;
}
// Needs to be here because called with only MoveId. Might refactor.
// Returns multiplicative modifier for draining moves (i.e. move.drain)
std::pair<int, int> BattleState::getDrain(MoveId move) {
  static const std::vector<MoveId> halfDrain = {
      MoveId::ABSORB,    MoveId::BITTERBLADE,    MoveId::DRAINPUNCH, MoveId::DREAMEATER,
      MoveId::GIGADRAIN, MoveId::HORNLEECH,      MoveId::LEECHLIFE,  MoveId::MATCHAGOTCHA,
      MoveId::MEGADRAIN, MoveId::PARABOLICCHARGE};
  if (std::find(halfDrain.begin(), halfDrain.end(), move) != halfDrain.end()) {
    return {1, 2};
  }
  if (move == MoveId::DRAININGKISS || move == MoveId::OBLIVIONWING) {
    return {3, 4};
  }
  return {0, 0};
}
// Handles the larger-scale idea of damage being dealt to a Pokemon: called often
// PokemonShowdown spreadDamage() or Battle.damage()
// Returns actual damage dealt.
// TODO: possible Weather+Immunity
// Do later: figure out what fastExit=true does and why target.hurtThisTurn = target.current_hp,
// MindBlown
DamageResultState BattleState::spreadDamage(DamageResultState damage, Pokemon &target,
                                            Pokemon &source, EffectKind effectKind,
                                            MoveId effectMove) {
  if (!target.current_hp) {
    damage.set_numeric(0);
    return damage;
  }
  // Struggle recoil is not affected by effects...
  // Start effects--
  // Possible Weather + immunity
  // What does fastExit=true do?
  damage.set_numeric(applyOnDamage(damage.damageDealt, target, source, effectKind, effectMove));
  // --End effects
  int damageDealt = applyDamage(damage.damageDealt, target);
  if (damageDealt) {
    target.wasHurtThisTurn = true;
    // target.hurtThisTurn = target.current_hp; // WTF?
  }
  if (effectKind == EffectKind::MOVE) {
    auto drain = getDrain(effectMove);
    if (damageDealt && drain.first > 0) {
      int drainAmt = damageDealt * drain.first / drain.second;
      source.heal(drainAmt, EffectKind::MOVE); // 'drain' effect
    }
  }
  damage.set_numeric(damageDealt);
  return damage;
  // Q: What's instafaint? A: MindBlown
}
// Might swap to just useMove()? Not sure what the wrapper is for in PokemonShowdown.
// TODO:
// - Team/Side and Scripted target logic
// - onAfterMoveSecondarySelf
// Do later:
// - onModifyTarget() and target checks
// - Pressure extra PP deduction
// - some mindBlownRecoil
// - ignoreImmunity
bool BattleState::useMoveInner(Pokemon &opp, Pokemon &user, MoveInstance &moveInst) {
  Move &move = moveInst.moveData;
  // user.lastMoveUsed = move -- ignore, only for Pokemon Stadium's Porygon
  // if(activeMove) priority tracking

  // Target selection
  Pokemon *target = &opp;
  if (moveInst.moveData.target == Target::SELF) {
    target = &user;
  }
  // set sourceEffect (for what?)
  // setActiveMove (optional given implementation?)
  // onModifyTarget -- only Comeuppance (Gen9) and MetalBurst
  // onModifyType():
  bool modifyTypeResult = moveInst.onModifyType(user);
  // onModifyMove():
  bool modifyMoveResult = moveInst.onModifyMove(user);
  if (!modifyTypeResult || !modifyMoveResult) {
    return false;
  }
  // TODO: Check no-target failure
  // Do later: Pressure extra PP deduction calc (for single battles, maybe do in runMove?)
  // onTryMove():
  if (!applyOnTryMove(*target, user, moveInst)) {
    // move.mindBlownRecoil = false;
    return false;
  }
  // TODO: Something about ignoreImmunity
  if (moveInst.alwaysSelfDestruct()) {
    faint(user); // Always-self-destructing moves faint
  }
  bool moveResult;
  switch (move.target) {
  case Target::ALL:
  case Target::ALLY_SIDE:
  case Target::FOE_SIDE:
  case Target::ALLY_TEAM: {
    // TODO
    // int damage = tryMoveHit(pokemon, move);
    // // Check NOT_FAIL for moveThisTurnResult?
    // // Note: should try to make tryMoveHit return 0 for undefined
    // moveResult = damage >= 0;
    moveResult = false;
    break;
  }
  case Target::NORMAL:
  case Target::SELF:
  case Target::SCRIPTED: {
    // TODO: have selfBoosts in trySpreadMoveHit() implementation for extra moveHit()
    // std::cout << "side " << user.side << " using move " << (int)moveInst.id << std::endl;
    moveResult = trySpreadMoveHit(*target, user, moveInst);
    break;
  }
  case Target::ADJACENT_ALLY: {
    // Fails in Singles
    moveResult = false;
    break;
  }
  }
  // TODO: potential moveHit() described above.
  // Faint-self if applicable
  if (!user.current_hp) {
    faint(user);
  }
  // Check move failure
  if (!moveResult) {
    applyOnMoveFail(*target, user, moveInst);
    return false;
  }
  // // TODO: Secondary effects: onAfterMoveSecondarySelf, onEmergencyExit
  // if (!move.negateSecondary && !(move.hasSheerForce && user.has_ability(Ability::SHEER_FORCE)) &&
  //     !move.moveData.futureMove) {
  //   int originalHP = user.current_hp;
  //   onAfterMoveSecondarySelf(target, user, moveInst);
  //   // if(not self-targeting or status) {
  //   //   if(originalHP above half but current HP at most half) {
  //   //     onEmergencyExit(user);
  //   //   }
  //   // }
  // }
  return true;
}
// Potentially a little more state management than useMoveInner? not sure
bool BattleState::useMove(Pokemon &target, Pokemon &user, MoveInstance &moveInst) {
  const bool moveResult = useMoveInner(target, user, moveInst);
  // If useMoveInner() doesn't change user.moveThisTurnResult, set that to its output
  return moveResult;
}
// Returns recoil damage to deal based on move.
int BattleState::calcRecoilDamage(int moveDmg, MoveInstance &moveInst, int userMaxHP) {
  // Struggle *rounds* 1/4 the HP
  if (moveInst.id == MoveId::STRUGGLE)
    return std::max(1, (userMaxHP + 2) / 4);
  // Chloroblast *rounds* half the HP
  if (moveInst.id == MoveId::CHLOROBLAST)
    return (userMaxHP + 1) / 2;
  auto [numer, denom] = moveInst.getRecoil();
  return std::max(1, moveDmg * numer / denom);
}
bool BattleState::checkHit(Pokemon &user, Pokemon &target, MoveInstance &moveInst, bool changeState) {
  Move &move = moveInst.moveData;
  // onTry() and onPrepareHit():
  if(!applyOnTry(user, target, moveInst)) {
    return false;
  }
  if(changeState && !applyOnPrepareHit(user, target, moveInst)) {
    // Q: What is NOT_FAIL? When moves return it, it actually seems to be a failure signal?
    // NOT_FAIL=="" has truthiness *false*
    // But by returning hitResult === NOT_FAIL; it implies it shouldn't be considered a failed
    // moveResult?
    return false;
  }
  bool hit = false;
  // From here, if any failures, don't keep going
  // hitStepInvulnerabilityEvent():
  if (move.id == MoveId::TOXIC && user.has_type(Type::POISON)) {
    hit = true;
  } else {
    hit = applyOnInvulnerability(target, moveInst);
  }
  // potential smartTarget adjustment
  // hitStepTryHitEvent():
  if (hit && changeState) {
    applyOnTryHitStep(target, user, moveInst);
  }
  // hitStepTypeImmunity():
  if (hit) {
    hit = false;
    // ignoreImmunity cases
    if (move.id == MoveId::BIDE || move.id == MoveId::FUTURESIGHT) {
      hit = true;
    } else if (move.id == MoveId::THOUSANDARROWS && move.type == Type::GROUND) {
      hit = true;
    } else if (move.category == MoveCategory::STATUS && move.id != MoveId::THUNDERWAVE) {
      // TODO: weird FutureSight hit case
      hit = true;
    } else {
      hit = runTypeImmunity(target, move.type);
    }
  }
  // hitStepTryImmunity():
  if (hit) {
    // The only natural Powder immunity is Grass-type
    if (moveInst.isPowder() && target.has_type(Type::GRASS) && target != user) {
      hit = false;
    } else if (!target.applyOnTryImmunity(user, move.id)) {
      hit = false;
    }
    // TODO
    // else if (move.pranksterBoosted && user.has_ability(Ability::PRANKSTER) &&
    //            target.side != user.side && target.has_type(Type::DARK)) {
    //   hit = false;
    // }
    else {
      hit = true;
    }
  }
  // hitStepAccuracy():
  if (hit && changeState) {
    auto accuracy = move.accuracy;
    bool is_ohko = moveInst.isOHKO();
    bool skip = false;
    if (is_ohko) { // OHKO case
      if (!target.isSemiInvulnerable()) {
        accuracy = {30};
        if (move.id == MoveId::SHEERCOLD && !user.has_type(Type::ICE)) {
          accuracy = {20};
        }
        if (user.lvl < target.lvl) {
          hit = false, skip = true;
        } else if (move.id == MoveId::SHEERCOLD && target.has_type(Type::ICE)) {
          hit = false, skip = true;
        } else {
          accuracy = {*accuracy + (user.lvl - target.lvl)};
        }
      }
    } else {
      if (accuracy != std::nullopt) {
        accuracy = {applyOnModifyAccuracy(target, user, moveInst, *accuracy)};
        accuracy = {moveInst.getBasicAcc(*accuracy, false, target, user)};
      }
    }
    if (!skip) {
      // Note: no move has flag alwaysHit
      if ((move.id == MoveId::TOXIC && user.has_type(Type::POISON)) ||
          (move.target == Target::SELF && move.category == MoveCategory::STATUS &&
           !target.isSemiInvulnerable()) ||
          applyOnAccuracy(target, user, moveInst)) {
        accuracy = std::nullopt;
      }
      hit = guaranteeHit ? true : (accuracy == std::nullopt) || math::randomChance(*accuracy, 100);
      if (!hit && !is_ohko && user.has_item(Item::BLUNDER_POLICY) && user.useItem(false, false)) {
        // TODO: Check that this is the correct effect kind
        ModifierTable boostTable = {{ModifierId::SPEED, 2}};
        user.boost(boostTable, EffectKind::NO_EFFECT, true);
      }
    }
  }
  // hitStepBreakProtect():
  if (hit && changeState) {
    if (moveInst.breaksProtect()) {
      bool broke = false;
      // TODO: Check single-target protects
      // TODO: Check side-effect protects
      if (broke) {
        // delete target.volatiles['stall']???
      }
    }
  }
  return hit;
}
// "try" and "hit" are because the move might miss for pre-condition/immunity/accuracy reasons
// "spreadMove" is because it includes the direct-damage dealing part of the move
// Returns whether or not the move did anything.
// TODO:
// - onTryHit()
// - entire hitStepBreakProtect
// Do later:
// - Figure out NOT_FAIL in applyOnTry and applyOnPrepareHit
// - FutureSight
// - PranksterBoost+Dark
// - Check BlunderPolicy boost effect flag
// - entire hitStepStealBoosts
// - smartTarget
// - gotAttacked
// - Check ending StompingTantrum case/logic
bool BattleState::trySpreadMoveHit(Pokemon &target, Pokemon &user, MoveInstance &moveInst) {
  Move &move = moveInst.moveData;
  bool hit = checkHit(user, target, moveInst, true);
  // Do later: hitStepStealBoosts(): only Marshadow's Spectral Thief
  // hitStepMoveHitLoop():
  int damageDealt = 0;
  if (hit) {
    int numHits = moveInst.getNumHits(user);
    moveInst.numHits = numHits;
    for (int hitNum = 1; hitNum <= numHits; hitNum++) {
      if (user.current_hp == 0)
        break;
      // why would damage.include(False)?
      // Make sure if fallen asleep (why check hit > 1?), does not keep hitting
      if (hitNum > 1 && user.status == Status::SLEEP && !moveInst.isSleepUsable())
        break;
      // Fail if all targets fainted
      if (target.current_hp == 0)
        break;
      moveInst.currentHitNum = hitNum; // update move.hit
      // more smartTarget logic
      // accuracy check (take evasion/modifyboosts into account)
      if (hitNum > 1 && moveInst.multiaccCheck(user)) {
        int acc = moveInst.getBasicAcc(*move.accuracy, true, target, user);
        acc = applyOnModifyAccuracy(target, user, moveInst, acc);
        // Not sure what alwaysHit is: it's never set to true
        if (!applyOnAccuracy(target, user, moveInst) && !math::randomChance(acc, 100)) {
          break;
        }
      }
      // is -1 if not numeric
      int moveDamageThisHit = moveHit(target, user, moveInst).damageDealt;
      damageDealt = moveDamageThisHit < 0 ? 0 : moveDamageThisHit;
      // std::cout << "move " << (int)move.id << " did " << damageDealt << " damage\n";
      moveInst.totalDamage += damageDealt;
      // Do later: mindBlownRecoil
      applyOnEach(EachEventKind::UPDATE);
    }
    if (moveInst.currentHitNum == 0) {
      hit = false;
    } else { // nullDamage is always false: ignore
      // Recoil damage
      // Fold Struggle recoil in -- it's like Chloroblast
      if ((moveInst.isRecoil() || moveInst.id == MoveId::CHLOROBLAST ||
           moveInst.id == MoveId::STRUGGLE) &&
          moveInst.totalDamage) {
        int hpBeforeRecoil = user.current_hp;
        int recoilDmg = calcRecoilDamage(moveInst.totalDamage, moveInst, user.stats.hp);
        spreadDamage(recoilDmg, user, user, EffectKind::RECOIL, MoveId::NONE);
        applyOnEmergencyExit(hpBeforeRecoil, user);
      }
      // some smartTarget logic?
      // TODO: target.gotAttacked: update attackedBy
      // timesAttacked: skip, only for RageFist (Gen9)
      if (!damageDealt)
        return 0;
      // onUpdate():
      applyOnEach(EachEventKind::UPDATE);
      // afterMoveSecondaryEffect() and EmergencyExit check
      // are negated by e.g. SheerForce.
      // negateSecondary seems to never be true, so ignore.
      if (!(moveInst.hasSheerForce && user.has_ability(Ability::SHEER_FORCE))) {
        applyOnAfterMoveSecondary(target, user, moveInst);
        if (target.current_hp) {
          applyOnEmergencyExit(target.current_hp + damageDealt, user);
        }
      }
    }
  }
  // TODO: Having to recharge or spending a turn trapped by another Pokemon's Sky Drop
  // does not boost StompingTantrum
  if (!hit)
    user.moveThisTurnFailed = false;
  return hit;
}
// source used for e.g. assigning duration of effect
// sourceEffect used for e.g. Yawn-
// TODO:
// - check sourceEffect flags
// - onHit()
bool BattleState::applySecondaryEffect(Pokemon &target, Pokemon &source, SecondaryEffect effect,
                                       MoveInstance &moveInst) {
  DamageResultState didSomething;
  switch (effect.kind) {
  case Secondary::STATUS: {
    // TODO: Check this is the right sourceEffect flag
    if (setStatus(effect.status, target, source, EffectKind::MOVE, moveInst)) {
      didSomething.set_succ();
    } else {
      didSomething.set_fail();
    }
    break;
  }
  case Secondary::BOOST: {
    if (!target.fainted) {
      // TODO: Check this is the right sourceEffect flag
      if (target.boost(effect.boosts, EffectKind::MOVE, false)) {
        didSomething.set_succ();
      } else {
        didSomething.set_fail();
      }
    }
    break;
  }
  case Secondary::SELFBOOST: {
    if (!target.fainted) {
      // TODO: Check this is the right sourceEffect flag
      if (target.boost(effect.selfBoosts, EffectKind::MOVE, true)) {
        didSomething.set_succ();
      } else {
        didSomething.set_fail();
      }
    }
    break;
  }
  case Secondary::VOLATILE: {
    if (addVolatile(effect.vol, target, source, moveInst.id)) {
      didSomething.set_succ();
    } else {
      didSomething.set_fail();
    }
    break;
  }
  case Secondary::CALLBACK: {
    // TODO: onHit()
    break;
  }
  default:
    break; // case NONE or SHEERFORCEBOOST
  }
  // Success so long as it didn't fail:
  // Either it actually succeeded or it didn't try anything.
  return !(didSomething.initialized && didSomething.fail);
}
// Applies the effects of a move actually hitting, including damage
// Expanded PokemonShowdown's spreadMoveHit, and removed pseudo-recursion
// TODO: more runMoveEffects
// Do later: Substitute case
DamageResultState BattleState::moveHit(Pokemon &target, Pokemon &user, MoveInstance &moveInst) {
  Move &move = moveInst.moveData;
  // PokemonShowdown's damage:
  DamageResultState dmgResult; // Returned value (updated as we go)
  // PokemonShowdown's target:
  bool subDamage = false; // If the move hit the substitute
  bool targeting = true;
  // Combines TryHitField, TryHitSide, and TryHit
  // Q: Why isn't this a MoveHitStep?
  // TODO: figure out difference with hitStepTryHitEvent
  // if (!applyOnTryHit(target, user, moveInst)) {
  //   dmgResult.set_fail();
  //   return dmgResult;
  // }

  // // Do later: Step 0 - Substitute check
  // if (move.target != Target::ALL && move.target != Target::ALLY_TEAM &&
  //       move.target != Target::ALLY_SIDE && move.target != Target::FOE_SIDE) {
  //     // only checks substitute condition
  //     damageDealt = applyOnTryPrimaryHit(target, user, moveInst);
  // }
  // if (damageDealt == HIT_SUBSTITUTE) {
  //   keepTargeting = true;
  //   // remove target: substitute blocked hit, no more effects applied to target
  // }
  // Do later: does no damage or dealt no Sub damage: keepTargeting = false;

  // Step 1 - getSpreadDamage(). Calls getDamage().
  if (!subDamage && targeting && move.category != MoveCategory::STATUS) {
    // battle.activeTarget = target;
    // Q: Can getDamage() return undefined or null? For now, assume no.
    dmgResult = getDamage(user, target, moveInst, defaultDmgOptions);
    if (dmgResult.initialized && dmgResult.fail) {
      targeting = false;
    }
    // std::cout << "damage: " << dmgResult.damageDealt << std::endl;
  }

  // Step 2 - spreadDamage()
  if (dmgResult.initialized && !dmgResult.fail) {
    if (subDamage || !targeting) {
      dmgResult.set_numeric(0);
    } else {
      dmgResult = spreadDamage(dmgResult, target, user, EffectKind::MOVE, move.id);
    }
    if (dmgResult.initialized && dmgResult.fail) {
      targeting = false;
    }
  }

  // Step 3 - Run the move's effects
  if (targeting) { // Includes subHit for self-destruct and self-switch
    DamageResultState didSomething;
    if (!subDamage) { // if(target)
      bool skip = false;
      bool hit = false;
      // boosts
      if (!target.fainted) {
        auto boosts = moveInst.getBoosts();
        if (!boosts.empty()) {
          hit = target.boost(boosts, EffectKind::NO_EFFECT, false);
        }
      }
      // heal
      if (!target.fainted) {
        auto healProp = moveInst.getHeal();
        if (healProp.first) {
          if (target.current_hp >= target.stats.hp) {
            // Fail if target at max HP
            dmgResult.set_fail();
            didSomething.set_fail();
            skip = true;
          }
          // Rounding
          int amtHealed = target.heal((target.stats.hp * healProp.first + healProp.second / 2) /
                                          healProp.second,
                                      EffectKind::NO_EFFECT);
          if (amtHealed < 0) {
            // Healing failed
            dmgResult.set_fail();
            didSomething.set_fail();
            skip = true;
          } else {
            didSomething.set_succ();
          }
        }
      }
      // status
      if (!skip) {
        Status status = moveInst.getStatus();
        if (status != Status::NO_STATUS) {
          hit = setStatus(status, target, user, EffectKind::NO_EFFECT, moveInst);
          if (!hit) {
            dmgResult.set_fail();
            didSomething.set_fail();
            skip = true;
          } else {
            didSomething.set_succ();
          }
        }
      }
      // volatile
      if (!skip) {
        VolatileId vol = moveInst.getVolatile();
        if (vol != VolatileId::NONE) {
          hit = addVolatile(vol, target, user, moveInst.id);
          if (!hit) {
            didSomething.set_fail();
          } else {
            didSomething.set_succ();
          }
        }
      }
      // TODO: sidecondition
      // TODO: weather
      // TODO: terrain
      // TODO: pseudoweather
      // TODO: forceswitch
      // onHit() (with onHitSide() and onHitField())
      if (!skip) {
        hit = applyOnHitMain(target, user, moveInst);
        if (!hit) {
          didSomething.set_fail();
        } else {
          didSomething.set_succ();
        }
      }
    }
    if (moveInst.isIfHitSelfDestruct() && dmgResult.initialized && !dmgResult.fail) {
      faint(user);
    }
    if (moveInst.isSelfSwitch()) {
      if (teams[user.side].pokemonLeft > 1) {
        didSomething.set_succ();
      } else {
        didSomething.set_fail();
      }
      if (!didSomething.fail) {
        user.switchFlag = true;
      }
    }
    // Fold didSomething into dmgResult
    if (!didSomething.initialized) {
      didSomething.set_succ();
    }
    if (didSomething.fail) {
      dmgResult.set_fail();
    } else if (didSomething.succ) {
      dmgResult.set_succ();
    } else {
      dmgResult.set_numeric(didSomething.damageDealt);
    }
    // Guaranteed to be initialized here
    if (dmgResult.fail) {
      targeting = false;
    }
  }

  // Skip: activeTarget preserving for Dancer
  // Step 4 - self drops
  if (targeting) {
    SecondaryEffect selfEffect = moveInst.getSelfEffect();
    if (selfEffect.kind != Secondary::NONE && !moveInst.selfDropped) {
      SecondaryEffect selfEffect = moveInst.getSelfEffect();
      // The self.boosts check is just 'cause the only random self-effect is stat boosting.
      int roll = math::random(100);
      if (selfEffect.chance == std::nullopt || roll < *selfEffect.chance) {
        applySecondaryEffect(user, user, selfEffect, moveInst);
      }
      if (moveInst.numHits) {
        moveInst.selfDropped = true;
      }
    }
  }
  // Step 5 - secondaries
  if (targeting) {
    auto secondaries = moveInst.getSecondaries();
    for (SecondaryEffect secondary : secondaries) {
      if (applyOnModifySecondaries(target, moveInst, secondary.kind)) {
        int roll = math::random(secondaryDenom);
        int chance = secondary.chance == std::nullopt ? -1 : *secondary.chance;
        if (secondary.kind == Secondary::BOOST || secondary.kind == Secondary::SELFBOOST) {
          chance %= 256;
        }
        if (chance < 0 || roll < chance) {
          if (secondary.kind == Secondary::SELFBOOST && !moveInst.selfDropped) {
            applySecondaryEffect(user, user, secondary, moveInst);
          } else {
            applySecondaryEffect(target, user, secondary, moveInst);
          }
        }
      }
    }
  }
  // End activeTarget preserving for Dancer

  // Step 6 - Force switch
  if (targeting) {
    if (moveInst.forcesSwitch()) {
      // forceSwitch():
      if (target.current_hp > 0 && user.current_hp > 0 && teams[target.side].pokemonLeft) {
        if (applyOnDragOut(target)) {
          target.forceSwitchFlag = true;
        }
      }
    }
    if (dmgResult.fail) {
      targeting = false; // Unused from here?
    }
  }

  // Step 7 - onDamagingHit(), onAfterHit(), and EmergencyExit check
  if (dmgResult.damageDealt > 0) { // By priority, implies is positive damage
    int originalHP = user.current_hp;
    applyOnDamagingHit(dmgResult.damageDealt, target, user, moveInst);
    applyOnAfterHit(target, user, moveInst);
    applyOnEmergencyExit(originalHP, user);
  }
  return dmgResult;
}
// Add to faintQueue
void BattleState::faint(Pokemon &pkmn) {
  if (pkmn.fainted || pkmn.faintQueued)
    return;
  // std::cout << "[BattleState::faint] side " << pkmn.side << std::endl;
  pkmn.current_hp = 0;
  // All faint switches happen at the end of the turn:
  pkmn.switchFlag = false;
  pkmn.faintQueued = true;
  faintQueue.push(&pkmn);
}
// Process faintQueue
// TODO: onFaint()
void BattleState::faintMessages() {
  for (; !faintQueue.empty(); faintQueue.pop()) {
    Pokemon &pokemon = *faintQueue.front();
    if (!pokemon.fainted) {
      teams[pokemon.side].pokemonLeft--;
      lastFaintSide = pokemon.side;
      // std::cout << "[BattleState::faintMessages] side " << pokemon.side << " dropped to " << teams[pokemon.side].pokemonLeft << " Pokemon left." << std::endl;
      // applyOnFaint(pokemon);
      // DestinyBond, Grudge, Sky Drop
      applyOnEnd(pokemon); // logging, PowerShift
      pokemon.clearVolatile(false);
      pokemon.fainted = true;
      pokemon.isActive = false;
      pokemon.isStarted = false;
    }
  }
}
} // namespace pkmn