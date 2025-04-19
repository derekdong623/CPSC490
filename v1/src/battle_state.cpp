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
// Could be called while non-switching Pokemon haven't moved yet.
void BattleState::set_switch_options() {
  for (int side = 0; side < 2; side++) {
    Team &team = teams[side];
    Pokemon &activePkmn = team.pkmn[team.activeInd];
    if (team.pokemonLeft && (!midTurn || activePkmn.switchFlag)) {
      team.choicesAvailable = {}; // Also erases move choices
      for (int j = 0; j < 6; j++) {
        Pokemon &candidate = team.pkmn[j];
        // Cannot switch into current active Pokemon
        if (team.activeInd != j && candidate.species != Species::NONE && !candidate.fainted) {
          // TODO: check that logic for locking/trapping in switchFlag is correct.
          team.choicesAvailable.swap[j] = true;
        }
      }
    }
  }
}
// Only called at end of turn, in set_choices().
void BattleState::set_move_options() {
  for (int side = 0; side < 2; side++) {
    Team &team = teams[side];
    Pokemon &activePkmn = team.pkmn[team.activeInd];
    for (int j = 0; j < 4; j++) {
      if (activePkmn.moves[j].id != MoveId::NONE && activePkmn.moves[j].pp > 0) {
        // TODO: locking, Struggle, etc.
        team.choicesAvailable.move[j] = true;
      }
    }
  }
}
// Called at the end of a turn to completely reset choices
void BattleState::set_choices() {
  set_switch_options(); // Removes other options
  // set_mega_options(); // Does not remove potential switch options
  set_move_options(); // Does not remove potential switch options
}
// Assumes move.power != std::nullopt
// Called by getSpreadDamage() and Substitute::onTryPrimaryHit() logic
DamageResultState BattleState::getDamage(Pokemon const &source, Pokemon const &target,
                                         MoveInstance &moveInst, DMGCalcOptions options) {
  DamageResultState ret;
  const Move &move = moveInst.moveData;
  // TODO: SPECIAL CASES
  // Some immunity check? (runImmunity)
  // OHKO: Fissure, Guillotine, Horn Drill, Sheer Cold (doesn't affect ice types).
  // damageCallback: Counter, Endeavor, Final Gambit, Guardian of Alola, Metal Burst, Mirror Coat,
  // Nature's Madness, Psywave, Ruination, Super Fang. Level: Night Shade, Seismic Toss. Flat dmg:
  // Dragon Rage, Sonic Boom.

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
  // overridePokemon -- just Offensive, from Foul Play
  Pokemon const &attacker = move.id == MoveId::FOULPLAY ? target : source;
  Pokemon const &defender = target;
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
  if (options.crit) {
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
// SwitchResult enum for directness
SwitchResult BattleState::switchIn(int side, int switchToInd, bool isDrag) {
  Team &team = teams[side];
  Pokemon &pokemon = team.pkmn[switchToInd]; // Use reference
  if (pokemon.isActive)
    return SwitchResult::FALSE;
  if (teams[side].activeInd >= 0) { // Otherwise is start of battle
    Pokemon &oldActive = team.pkmn[team.activeInd];
    if (oldActive.current_hp > 0) { // unfaintedActive
      oldActive.beingCalledBack = true;
      // TODO: Baton Pass flag set
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
      // oldActive.illusion = false;
      applyOnEnd(oldActive);
      // Do later: Baton Pass copyVolatile
      oldActive.clearVolatile(true);
    }
    oldActive.isActive = false;
    oldActive.isStarted = false;
    oldActive.usedItemThisTurn = false;
    oldActive.statsRaisedThisTurn = false;
    oldActive.statsLoweredThisTurn = false;
    // if(oldActive.fainted) oldActive.status = Status::NO_STATUS;
    // Switch positions for potential DragIn shenanigans
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
bool BattleState::startBattle() {
  // start() calls turnLoop() which calls runAction('start').
  // Only endTurn() is run from applyAtEndOfAction
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
  // Runs onStart for each *pokemon*, not just active?
  for (int side = 0; side < 2; side++) {
    for (int i = 0; i < 6; i++) {
      if (teams[side].pkmn[i].species != Species::NONE) {
        applyOnStart(teams[side].pkmn[i]);
      }
    }
  }
  endTurn();
  midTurn = false;
  return true;
}
void sortMoveActions(std::vector<MoveActionPriority> &moveActions) {
  if (moveActions.size() >= 2) { // Should be equal to 2, then
    if (moveActions[1] < moveActions[0] ||
        (!(moveActions[0] < moveActions[1]) && math::random(2))) {
      std::swap(moveActions[0], moveActions[1]);
    }
  }
}
void sortSwitchActions(std::vector<SwitchAction> &switchActions) {
  if (switchActions.size() >= 2) { // Should be equal to 2, then
    if (switchActions[1] < switchActions[0] ||
        (!(switchActions[0] < switchActions[1]) && math::random(2))) {
      std::swap(switchActions[0], switchActions[1]);
    }
  }
}
// Validate move selection and get struct holding the move's priority, user's speed, and any
// fractional priority
MoveActionPriority BattleState::getMoveActionFromChoice(int side) {
  MoveActionPriority ret;
  Team &team = teams[side];
  Pokemon &pokemon = team.pkmn[team.activeInd];
  ret.moveInd = -1;
  for (int i = 0; i < 4; i++) {
    if (team.choicesAvailable.move[i]) {
      if (ret.moveInd >= 0) {
      } // Error: multiple moves selected
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
    // onModifyPriority callback:
    if (pokemon.ability == Ability::GALE_WINGS && move.type == Type::FLYING &&
        pokemon.current_hp == pokemon.stats.hp) {
      ret.priority++;
    }
    if (pokemon.ability == Ability::PRANKSTER && move.category == MoveCategory::STATUS) {
      ret.priority++;
    }
    // if(pokemon.ability == Ability::TRIAGE && move.flags['heal']) {
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
      } // Error: multiple Pokemon selected
      ret.newPokeInd = i;
    }
  }
  if (ret.newPokeInd >= 0) { // A move was selected
    if (ret.newPokeInd == team.activeInd) {
    } // Error: chose to swap to current active Pokemon
    ret.side = side;
    ret.speed = pokemon.effectiveSpeed;
  }
  return ret;
}
// Main call exposed by API.
// Note that it manipulates and returns a *copy* of itself.
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
void BattleState::endTurn() {
  turnNum++;
  // lastSuccessfulMoveThisTurn = MoveId::NONE;
  for (int side = 0; side < 2; side++) {
    Pokemon &pkmn = teams[side].pkmn[teams[side].activeInd];
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
    // for (int i = 0; i < 4; i++) {
    //   MoveSlot &slot = pkmn.moves[i];
    //   slot.disabled = false;
    //   slot.disabledSource = null;
    // }
    // applyOnDisableMove(pkmn);
    // for (int i = 0; i < 4; i++) {
    //   const activeMove = this.dex.getActiveMove(moveSlot.id);
    //   applyOnDisableMove(activeMove, pkmn);
    //   if (activeMove.flags['cantusetwice'] && pokemon.lastMove?.id === moveSlot.id) {
    //     pkmn.disableMove(pokemon.lastMove.id);
    //   }
    // }
    // Do later: update attackedBy
    // TRAPPING
    pkmn.trapped = false;
    applyOnTrapPokemon(pkmn);
    // applyOnMaybeTrapPokemon(pkmn); // No such callback?
    // TODO: cancel switch choices?
    if (pkmn.fainted)
      continue;
    // // Not sure what staleness means
    // const staleness = pokemon.volatileStaleness || pokemon.staleness;
    // if (staleness) sideStaleness = sideStaleness === 'external' ? sideStaleness : staleness;
    // stalenessBySide.push(sideStaleness);
    teams[side].faintedLastTurn = teams[side].faintedThisTurn;
    teams[side].faintedThisTurn = false;
    pkmn.activeTurns++;
  }
}
std::optional<bool> BattleState::applyAtEndOfAction(ActionKind action) {
  // TODO: phaze
  // clearActiveMove(failed=false);
  faintMessages();
  if (battle_ended)
    return {false};
  // checkFainted: only on residual
  if (action == ActionKind::RESIDUAL) {
    for (int side = 0; side < 2; side++) {
      Pokemon &activePkmn = teams[side].pkmn[teams[side].activeInd];
      if (activePkmn.fainted)
        activePkmn.switchFlag = true;
    }
  }
  applyOnEach(EachEventKind::UPDATE);
  // TODO: run applyOnEmergencyExit for any residual-affected Pokemon

  // TODO
  // if(action == EndOfActionKind::RESIDUAL) {
  //   applyOnEmergencyExit(originalHP, pokemon);
  // }

  for (int side = 0; side < 2; side++) {
    Pokemon &activePkmn = teams[side].pkmn[teams[side].activeInd];
    bool toSwitch = activePkmn.switchFlag;
    if (toSwitch) {
      if (teams[side].pokemonLeft) {
        if (activePkmn.current_hp && !activePkmn.skipBeforeSwitchOutEventFlag) {
          applyOnBeforeSwitchOut(activePkmn);
          activePkmn.skipBeforeSwitchOutEventFlag = true;
          faintMessages(); // Pokemon may have fainted in BeforeSwitchOut
          if (battle_ended)
            return {false};
          if (activePkmn.fainted) {
            // Might've swapped which Pokemon is active
            toSwitch = teams[side].pkmn[teams[side].activeInd].switchFlag;
          }
        }
      } else {
        activePkmn.switchFlag = false;
        toSwitch = false;
      }
    }
  }
  // Post-faint switches: request simultaneously
  set_switch_options();
  return std::nullopt;
}
// Returns false if either the battle is unstarted or ended.
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
  if (currentAction <= ActionKind::SWITCH) {
    currentAction = ActionKind::SWITCH;
    // Create SwitchActions
    std::vector<SwitchAction> switchActions;
    for (int side = 0; side < 2; side++) {
      SwitchAction switchAct = getSwitchActionFromChoice(side);
      if (switchAct.newPokeInd >= 0) {
        switchActions.push_back(switchAct);
      }
    }
    // NB: 'switch' sets up callbacks (e.g. beforeTurnMove) and calls 'runSwitch'
    // 'runSwitch' executes available switch choices in order
    sortSwitchActions(switchActions);
    for (size_t i = 0; i < switchActions.size(); i++) {
      int side = switchActions[i].side;
      // Reset choice
      teams[side].choicesAvailable = Choice{};
      switchIn(side, switchActions[i].newPokeInd, false);
      auto res = applyAtEndOfAction(ActionKind::SWITCH);
      if (res != std::nullopt)
        return *res;
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
      // Reset choice
      team.choicesAvailable = Choice{};
      // Cannot use move if switched out e.g. by Whirlwind or EjectButton
      if (pokemon.isActive) {
        runMove(moveSlot, pokemon, otherTeam.pkmn[otherTeam.activeInd]);
        auto res = applyAtEndOfAction(ActionKind::MOVE);
        if (res != std::nullopt)
          return *res;
      }
    }
  }
  if (currentAction <= ActionKind::RESIDUAL) {
    // 'residual' action: updateSpeed() and applyOnResidual() for each active Pokemon in speed order
    // clearActiveMove(failed=true);
    updateSpeed();
    applyOnEach(EachEventKind::RESIDUAL);
    auto res = applyAtEndOfAction(ActionKind::RESIDUAL);
    if (res != std::nullopt)
      return *res;
  }
  endTurn();
  currentAction = ActionKind::NONE;
  midTurn = false;
  set_choices();
  return true;
}
// Update winner if found
int BattleState::checkWin(int lastFaintSide) {
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
void BattleState::runMove(MoveSlot &moveSlot, Pokemon &user, Pokemon &target) {
  user.activeMoveActions++;
  // instantiate the moveInst from the moveSlot
  MoveInstance moveInst = MoveInstance(moveDict.dict[moveSlot.id]);
  // Do later: potential Encore move replacement/retargeting (onOverrideAction) with Prankster boost
  // case onBeforeMove():
  bool stompingTantrumFail = applyOnBeforeMoveST(user);
  bool willTryMove = !stompingTantrumFail && applyOnBeforeMove(user);
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
  // TODO: Update pokemon.lastMove, lastMoveTargetLoc, and moveThisTurn
  // pokemon.lastDamage is never read: skip
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
  // user.lastMoveTargetLoc = ...
  // user.moveThisTurn = ...

  bool moveDidSomething = useMove(target, user, moveInst);
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
  // applyOnNegateImmunity(target, typ)
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
  return getTypeMod(target, typ) >= 0;
}
// false for immune, true for not-immune
bool BattleState::runStatusImmunity(Pokemon &target, Status status) {
  if (target.fainted)
    return false;
  if (status == Status::NO_STATUS)
    return true;

  // Natural type immunities
  if (target.has_type(Type::ELECTRIC) && status == Status::PARALYSIS) {
    return false;
  } else if (target.has_type(Type::FIRE) && status == Status::BURN) {
    return false;
  } else if (target.has_type(Type::ICE) && status == Status::FREEZE) {
    return false;
  } else if ((target.has_type(Type::POISON) || target.has_type(Type::STEEL)) &&
             (status == Status::PARALYSIS || status == Status::TOXIC)) {
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
bool BattleState::isGrounded(Pokemon &pokemon, bool negateImmunity) {
  if (gravity)
    return true;
  if (pokemon.ingrained)
    return true;
  if (pokemon.smackeddown)
    return true;
  //  TODO: ignoringItem
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
  // applyOnStart() -- Status version
  if (target.status == Status::SLEEP) {
    // Do later: cure nightmare
  } else if (target.status == Status::FREEZE) {
    // formeChange Shaymin-Sky back to Shaymin
  } else if (target.status == Status::TOXIC) {
    target.toxicStage = 0;
  }
  // applyOnAfterSetStatus()
  // TODO:
  if (target.has_ability(Ability::SYNCHRONIZE)) {
    // If source != target & effectKind != ToxicSpike && !slp && !frz:
    // source.setStatus(status, source, *this, EffectKind::SYNCHRONIZE);
  }
  if (target.has_item(Item::LUM_BERRY)) {
    target.useItem(true, false);
  }
  return true;
}
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
    return true;
  } else {
    return false;
  }
}
// Actually apply the damage (capped by current HP)
// Called by spreadDamage() and directDamage()
int BattleState::applyDamage(int damage, Pokemon &target) {
  if (target.current_hp <= 0 || damage <= 0)
    return 0;
  target.current_hp -= damage;
  if (target.current_hp < 0) {
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
// Returns multiplicative modifier for draining moves (i.e. move.drain)
std::pair<int, int> getDrain(MoveId move) {
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
// Handles the larger-scale idea of healing a Pokemon: called often
// Returns actual damage healed, or -1 for failure
int BattleState::heal(int damage, Pokemon &target, EffectKind effectKind) {
  damage = target.applyOnTryHeal(damage, effectKind);
  if (!damage)
    return damage;
  if (!target.current_hp || !target.isActive)
    return 0;
  int healAmt = target.applyHeal(damage);
  // applyOnHeal(healAmt, target, source, effect); // Does nothing it seems
  return healAmt;
}
// Handles the larger-scale idea of damage being dealt to a Pokemon: called often
// Returns actual damage dealt
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
      heal(drainAmt, source, EffectKind::MOVE); // 'drain' effect
    }
  }
  damage.set_numeric(damageDealt);
  return damage;
  // Q: What's instafaint? A: MindBlown
}
bool BattleState::useMoveInner(Pokemon &opp, Pokemon &user, MoveInstance &moveInst) {
  Move &move = moveInst.moveData;
  // user.lastMoveUsed = move -- ignore, only for Pokemon Stadium's Porygon
  // if(activeMove) priority tracking

  // Target selection
  Pokemon &target = opp;
  if (moveInst.moveData.target == Target::SELF) {
    target = user;
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
  if (!applyOnTryMove(target, user, moveInst)) {
    // move.mindBlownRecoil = false;
    return false;
  }
  // TODO: Something about ignoreImmunity
  // Always-self-destructing moves faint
  if (moveInst.alwaysSelfDestruct()) {
    faint(user);
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
    moveResult = trySpreadMoveHit(target, user, moveInst);
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
    applyOnMoveFail(target, user, moveInst);
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
bool BattleState::trySpreadMoveHit(Pokemon &target, Pokemon &user, MoveInstance &moveInst) {
  Move &move = moveInst.moveData;
  // onTry() and onPrepareHit()
  if (!applyOnTry(user, target, moveInst) || !applyOnPrepareHit(user, target, moveInst)) {
    // Q: What is NOT_FAIL? When moves return it, it actually seems to be a failure signal?
    // NOT_FAIL=="" has truthiness *false*
    // But by returning hitResult === NOT_FAIL; it implies it shouldn't be considered a failed
    // moveResult?
    return false;
  }
  // Check pre-condition for move. Not sure what the difference between the two is.
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
  if (hit) {
    // TODO: onTryHit():
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
  if (hit) {
    auto accuracy = move.accuracy;
    bool is_ohko = moveInst.isOHKO();
    bool skip = false;
    if (is_ohko) {
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
      hit = (accuracy == std::nullopt) || math::randomChance(*accuracy, 100);
      if (!hit && !is_ohko && user.has_item(Item::BLUNDER_POLICY) && user.useItem(false, false)) {
        // TODO: Check that this is the correct effect kind
        user.boost({{ModifierId::SPEED, 2}}, EffectKind::NO_EFFECT);
      }
    }
  }
  // hitStepBreakProtect():
  if (hit) {
    if (moveInst.breaksProtect()) {
      bool broke = false;
      // TODO: Check single-target protects
      // TODO: Check side-effect protects
      if (broke) {
        // delete target.volatiles['stall']???
      }
    }
  }
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
      moveInst.totalDamage += damageDealt;
      // mindBlownRecoil
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
        applyDamage(calcRecoilDamage(moveInst.totalDamage, moveInst, user.stats.hp), user);
        applyOnEmergencyExit(hpBeforeRecoil, user);
      }
      // some smartTarget logic?
      // TODO: target.gotAttacked: update attackedBy
      // timesAttacked: skip, only for RageFist (Gen9)
      if (!damageDealt)
        return 0;
      // onUpdate():
      applyOnEach(EachEventKind::UPDATE);
      // afterMoveSecondaryEffect() and...
      // EmergencyExit check is negated by e.g. SheerForce.
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
// TODO: Give SparklingAria dustproof, getting it through ShieldDust
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
      if (target.boost(effect.boosts, EffectKind::MOVE)) {
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
      if (target.boost(effect.selfBoosts, EffectKind::MOVE)) {
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
DamageResultState BattleState::moveHit(Pokemon &target, Pokemon &user, MoveInstance &moveInst) {
  Move &move = moveInst.moveData;
  // PokemonShowdown's damage:
  DamageResultState dmgResult; // Returned value (updated as we go)
  // PokemonShowdown's target:
  bool subDamage = false; // If the move hit the substitute
  bool targeting = true;
  // Combines HitField, HitSide, and Hit
  // Q: Why isn't this a MoveHitStep?
  if (!applyOnTryHit(target, user, moveInst)) {
    dmgResult.set_fail();
    return dmgResult;
  }

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
      // TODO
      bool skip = false;
      bool hit = false;
      // boosts
      if(!target.fainted){
        auto boosts = moveInst.getBoosts();
        if(!boosts.empty()) {
          hit = target.boost(boosts, EffectKind::NO_EFFECT);
        }
      }
      // heal
      if(!target.fainted) {
        auto healProp = moveInst.getHeal();
        if(healProp.first) {
          if(target.current_hp >= target.stats.hp) {
            // Fail if target at max HP
            dmgResult.set_fail();
            didSomething.set_fail();
            skip = true;
          }
          // Rounding
          int amtHealed = heal((target.stats.hp * healProp.first + healProp.second / 2) / healProp.second, target, EffectKind::NO_EFFECT);
          if(amtHealed < 0) {
            // Healing failed
            dmgResult.set_fail();
            didSomething.set_fail();
            skip = true;
          }
          didSomething.set_succ();
        }
      }
      // status
      if(!skip) {
        Status status = moveInst.getStatus();
        if(status != Status::NO_STATUS) {
          hit = setStatus(status, target, user, EffectKind::NO_EFFECT, moveInst);
          if(!hit) {
            dmgResult.set_fail();
            didSomething.set_fail();
            skip=true;
          }
          didSomething.set_succ();
        }
      }
      // volatile
      // sidecondition
      // weather
      // terrain
      // pseudoweather
      // forceswitch
      // onHit
    }
    if (moveInst.isIffHitSelfDestruct() && dmgResult.initialized && !dmgResult.fail) {
      faint(user);
    }
    if (moveInst.isSelfSwitch()) {
      if (teams[user.side].pokemonLeft > 1) {
        didSomething.set_succ();
      } else {
        didSomething.set_fail();
      }
      if (!didSomething.fail) {
        // user.switchFlag = Move.id;
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

  // activeTarget preserving for Dancer
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
        int roll = math::random(100);
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
      // Call forceSwitch()
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
  pkmn.current_hp = 0;
  pkmn.faintQueued = true;
  faintQueue.push(&pkmn);
}
// Process faintQueue
void BattleState::faintMessages() {
  for (; !faintQueue.empty(); faintQueue.pop()) {
    Pokemon &pokemon = *faintQueue.front();
    if (!pokemon.fainted) {
      teams[pokemon.side].pokemonLeft--;
      // applyOnFaint(pokemon); // DestinyBond, Grudge, Sky Drop
      applyOnEnd(pokemon); // logging, PowerShift,
      pokemon.clearVolatile(false);
      pokemon.fainted = true;
      pokemon.isActive = false;
      pokemon.isStarted = false;
    }
  }
}
} // namespace pkmn