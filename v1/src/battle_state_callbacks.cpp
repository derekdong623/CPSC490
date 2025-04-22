#include "battle_state.hpp"
#include "utils.hpp"

#include <optional>
namespace pkmn {
void BattleState::applyOnTrapPokemon(Pokemon &pokemon) {
  // PRIORITY 0
  if (pokemon.trappedCondition) {
    pokemon.tryTrap();
  }
  // Do later: partial trapping
  // Do later: FairyLock
  // Do later: Octolock
  if (pokemon.ingrained) {
    pokemon.tryTrap();
  }
  if (pokemon.noretreat) {
    pokemon.tryTrap();
  }
  // PRIORITY -10
  if (pokemon.has_item(Item::SHED_SHELL)) {
    pokemon.trapped = false;
    // pokemon.maybeTrapped = false;
  }
}
// Just Pursuit
void BattleState::applyOnBeforeSwitchOut(Pokemon &pokemon) {
  // // Do later: Pursuit
  // if (pokemon.pursuited) {
  //   pokemon.removeVolatile(VolatileId::DESTINY_BOND);
  //   // No loop: only one possible foe to have been Pursuited by
  //   // Do later: Remove Pursuit from MoveActions to run
  //   if(!pokemon.pursuitedBy.current_hp) return;
  //   // Run through each action in queue to check if the Pursuit user is supposed to Mega Evolve
  //   this
  //   // turn. If it is, then Mega Evolve before moving.
  //   if (pokemon.pursuitedBy.willMega) {
  //     runMegaEvo(pokemon.pursuitedBy);
  //   }
  //   runMove(pursuitMoveSlot, pokemon.pursuitedBy, pokemon);
  // }
}
// Just NaturalCure and Regenerator
void BattleState::applyOnSwitchOut(Pokemon &pokemon) {
  switch (pokemon.ability) {
  case Ability::NATURAL_CURE: {
    if (pokemon.status != Status::NO_STATUS) {
      pokemon.status = Status::NO_STATUS;
    }
    break;
  }
  case Ability::REGENERATOR: {
    pokemon.applyHeal(pokemon.stats.hp / 3);
    break;
  }
  default:
    break;
  }
}
// Just Illusion
void BattleState::applyOnBeforeSwitchIn(Pokemon &pokemon) {
  // Do later: Illusion
}
void BattleState::applyOnWeatherChange(Pokemon &pokemon) {
  // Do later
  switch (pokemon.ability) {
    // case Ability::FLOWER_GIFT:
    // case Ability::FORECAST:
    // case Ability::ICE_FACE:
  default:
    break;
  }
}
// TODO: figure out what exactly e.g. onSwitchInPriority does
void BattleState::applyOnSwitchIn(Pokemon &pokemon) {
  // PRIORITY 2
  switch (pokemon.ability) {
  case Ability::NEUTRALIZING_GAS: {
    // Do later
    break;
  }
  default:
    break;
  }
  // PRIORITY 0
  switch (pokemon.ability) {
  case Ability::AIR_LOCK:
  case Ability::CLOUD_NINE: {
    // Screw it, just run onStart directly
    pokemon.weatherSuppressEnding = false;
    applyOnWeatherChange(pokemon);
    break;
  }
  case Ability::IMPOSTER: {
    // Do later
    break;
  }
  default:
    break;
  }
  if (pokemon.status == Status::TOXIC) {
    pokemon.toxicStage = 0;
  }
  // Do later: healReplacement from Healer
  // PRIORITY -1
  switch (pokemon.item) {
  case Item::BLUE_ORB: {
    // Do later
    // if (pokemon.isActive && pokemon.name == PokeName::KYOGRE && !pokemon.transformed) {
    //   pokemon.formeChange(PokeName::PRIMAL_KYOGRE, isPermanent=true);
    // }
    // break;
  }
  case Item::RED_ORB: {
    // Do later
    // if (pokemon.isActive && pokemon.name == PokeName::GROUDON && !pokemon.transformed) {
    //   pokemon.formeChange(PokeName::PRIMAL_GROUDON, isPermanent=true);
    // }
    // break;
  }
  default:
    break;
  }
  // Do later: move conditions
  // HealingWish, LunarDance, Spikes, StealthRock, StickyWeb, ToxicSpikes
  // Note StickyWeb has Gen8 update
}
// All onBasePower callback logic
int BattleState::applyOnBasePower(int basePower, const Pokemon &source, const Pokemon &target,
                                  const Move &move) {
  // See spreadsheet for progress
  static const std::vector<MoveId> punchMove = {
      MoveId::BULLETPUNCH,  MoveId::COMETPUNCH,   MoveId::DIZZYPUNCH,  MoveId::DOUBLEIRONBASH,
      MoveId::DRAINPUNCH,   MoveId::DYNAMICPUNCH, MoveId::FIREPUNCH,   MoveId::FOCUSPUNCH,
      MoveId::HAMMERARM,    MoveId::HEADLONGRUSH, MoveId::ICEHAMMER,   MoveId::ICEPUNCH,
      MoveId::MACHPUNCH,    MoveId::MEGAPUNCH,    MoveId::METEORMASH,  MoveId::PLASMAFISTS,
      MoveId::POWERUPPUNCH, MoveId::SHADOWPUNCH,  MoveId::SKYUPPERCUT, MoveId::SURGINGSTRIKES,
      MoveId::THUNDERPUNCH, MoveId::WICKEDBLOW};
  if (std::find(punchMove.begin(), punchMove.end(), move.id) != punchMove.end()) {
    if (source.ability == Ability::IRON_FIST) {
      basePower = applyModifier(basePower, 4915, 4096); // 1.2
    }
    if (source.item == Item::PUNCHING_GLOVE) {
      basePower = applyModifier(basePower, 4505, 4096); // 1.1
    }
  }
  // TODO: finish callback
  return basePower;
}
// Base power adjustments
int BattleState::applyBasePowerCallback(int basePower, Pokemon const &attacker,
                                        Pokemon const &defender, MoveInstance const &moveInst) {
  switch (moveInst.id) {
  case MoveId::ACROBATICS: {
    if (attacker.item == Item::NO_ITEM) {
      return basePower * 2;
    }
    break;
  }
  case MoveId::ASSURANCE: {
    if (defender.wasHurtThisTurn) {
      return basePower * 2;
    }
    break;
  }
  case MoveId::AVALANCHE:
  case MoveId::REVENGE: {
    // if(attacker.wasDamagedByDefenderThisTurn) {
    //   return basePower * 2;
    // }
    break;
  }
  case MoveId::HEX:
  case MoveId::INFERNALPARADE: {
    if (defender.status != Status::NO_STATUS || defender.has_ability(Ability::COMATOSE)) {
      return basePower * 2;
    }
    break;
  }
  case MoveId::SMELLINGSALTS: {
    if (defender.status == Status::PARALYSIS) {
      return basePower * 2;
    }
    break;
  }
  case MoveId::WAKEUPSLAP: {
    if (defender.status == Status::SLEEP || defender.has_ability(Ability::COMATOSE)) {
      return basePower * 2;
    }
    break;
  }

  // case MoveId::RISINGVOLTAGE:{
  //   if(terrain == Terrain::ELECTRIC && defender.isGrounded()) {
  //     return basePower * 2;
  //   }
  //   break;}
  case MoveId::STOMPINGTANTRUM:
  case MoveId::TEMPERFLARE: {
    if (attacker.moveLastTurnFailed) {
      return basePower * 2;
    }
    break;
  }

  // HP-BASED
  case MoveId::DRAGONENERGY:
  case MoveId::ERUPTION:
  case MoveId::WATERSPOUT: {
    return basePower * attacker.current_hp / attacker.stats.hp;
  }
  case MoveId::CRUSHGRIP:
  case MoveId::WRINGOUT: {
    int hp = defender.current_hp;
    int maxHP = defender.stats.hp;
    return std::max((hp * 4096 / maxHP * 100 * 120 + 2048 - 1) / 4096 / 100, 1);
  }
  case MoveId::HARDPRESS: {
    int hp = defender.current_hp;
    int maxHP = defender.stats.hp;
    return std::max((hp * 4096 / maxHP * 100 * 100 + 2048 - 1) / 4096 / 100, 1);
  }
  case MoveId::FLAIL:
  case MoveId::REVERSAL: {
    int ratio = std::max(attacker.current_hp * 48 / attacker.stats.hp, 1);
    if (ratio < 2) {
      return 200;
    } else if (ratio < 5) {
      return 150;
    } else if (ratio < 10) {
      return 100;
    } else if (ratio < 17) {
      return 80;
    } else if (ratio < 33) {
      return 40;
    }
    return 20;
  }

  // SPEED-BASED
  case MoveId::ELECTROBALL: {
    int defSpeed = defender.getStat(ModifierId::SPEED, true, true);
    int ratio = defSpeed > 0 ? attacker.getStat(ModifierId::SPEED, true, true) / defSpeed : 0;
    static const std::vector<int> electroBallBP = {40, 60, 80, 120, 150};
    return electroBallBP[std::min(ratio, 4)];
  }
  case MoveId::GYROBALL: {
    int attSpeed = attacker.getStat(ModifierId::SPEED, true, true);
    return attSpeed > 0
               ? std::min(150, 25 * defender.getStat(ModifierId::SPEED, true, true) / attSpeed + 1)
               : 1;
  }
  // HAPPINESS-BASED: haven't implemented happiness yet
  // case MoveId::RETURN:{
  // return std::max(1, 10 * attacker.happiness / 25);}
  // case MoveId::FRUSTRATION:{
  // return std::max(1, ((255 - attacker.happiness) * 10) / 25);}

  // WEIGHT-BASED: haven't implemented weight yet
  // case MoveId::GRASSKNOT:
  // case MoveId::LOWKICK:
  // case MoveId::HEATCRASH:
  // case MoveId::HEAVYSLAM:

  // MULTIPLIER-BASED
  case MoveId::ECHOEDVOICE: {
    return basePower * echoedVoiceMultiplier;
  }
  case MoveId::TRIPLEAXEL:
  case MoveId::TRIPLEKICK: {
    return basePower * moveInst.currentHitNum;
  }
  case MoveId::TRUMPCARD: {
    int pp = moveInst.moveData.pp;
    static const std::vector<int> trumpCardBP = {200, 80, 60, 50, 40};
    return trumpCardBP[std::min(4, pp)];
  }
  // // Skip for now
  // case MoveId::SPITUP:
  // case MoveId::FURYCUTTER:
  // case MoveId::ICEBALL:
  // case MoveId::ROLLOUT:

  // MISC
  case MoveId::PURSUIT: {
    if (defender.beingCalledBack || defender.switchFlag) {
      return basePower * 2;
    }
    break;
  }
  case MoveId::BOLTBEAK:
  case MoveId::FISHIOUSREND: {
    // if(target.newlySwitched || target.willMove) {
    //   return basePower * 2;
    // }
    break;
  }
  case MoveId::PAYBACK: {
    // if(!(target.newlySwitched || target.willMove)) {
    //   return basePower * 2;
    // }
    break;
  }
  // case MoveId::STOREDPOWER:
  // case MoveId::POWERTRIP:{
  //   return basePower + 20 * attacker.numPositiveBoosts();}
  // case MoveId::PUNISHMENT:{
  //   return std::min(200, 60 + 20 * defender.numPositiveBoosts());}
  // case MoveId::WATERSHURIKEN: // Greninja-Ash special case
  // case MoveId::ROUND:
  // // Pledge effects won't happen in single battle
  // case MoveId::FIREPLEDGE:
  // case MoveId::GRASSPLEDGE:
  // case MoveId::WATERPLEDGE:
  case MoveId::BEATUP: {
    // Complicated stuff
    break;
  }
  default:
    break;
  }
  return basePower;
}
// Modify crit true/false result directly.
bool BattleState::applyOnCriticalHit(bool crit, Pokemon const &target, Pokemon const &source,
                                     MoveInstance const &moveInst) {
  // BattleArmor, ShellArmor
  if (target.has_ability(Ability::BATTLE_ARMOR) || target.has_ability(Ability::SHELL_ARMOR))
    return false;
  // Disguise, IceFace
  if ((target.species == Species::EISCUE && target.has_ability(Ability::ICE_FACE) &&
       moveInst.moveData.category == MoveCategory::PHYSICAL) ||
      (target.species == Species::MIMIKYU && target.has_ability(Ability::DISGUISE))) {
    // if(substitute doesn't block it already)
    // if(not already immune to attack)
    // return false;
  }
  // LuckyChant
  if (teams[target.side].luckyChant >= 0) {
    // std::cerr << "lucky chant " << teams[target.side].luckyChant << " blocked crit.\n";
    return false;
  }
  return crit;
}
// NB: all but Hustle use chainModify...what's the difference?
int BattleState::applyOnModifyAtk(int attack, Pokemon const &attacker, Pokemon const &defender,
                                  Move const &move) {
  // PRIORITY 5
  // APPLIES TO ONLY ATK
  // Guts
  if (attacker.has_ability(Ability::GUTS) && attacker.status != Status::NO_STATUS) {
    attack = applyModifier(attack, 3, 2);
  }
  // HugePower, PurePower
  if (attacker.has_ability(Ability::HUGE_POWER) || attacker.has_ability(Ability::PURE_POWER)) {
    attack *= 2;
  }
  // Hustle
  if (attacker.has_ability(Ability::HUSTLE)) {
    attack = applyModifier(attack, 3, 2);
  }
  // // SlowStart
  // if (attacker.slowstart) {
  //   attack /= 2;
  // }
  // APPLIES TO BOTH
  // Defeatist
  if (attacker.has_ability(Ability::DEFEATIST) && attacker.current_hp * 2 <= attacker.stats.hp) {
    attack /= 2;
  }
  // Blaze, Overgrow, Torrent, Swarm
  if (attacker.current_hp * 3 <= attacker.stats.hp) {
    if ((attacker.has_ability(Ability::BLAZE) && move.type == Type::FIRE) ||
        (attacker.has_ability(Ability::OVERGROW) && move.type == Type::GRASS) ||
        (attacker.has_ability(Ability::TORRENT) && move.type == Type::WATER) ||
        (attacker.has_ability(Ability::SWARM) && move.type == Type::BUG)) {
      attack = applyModifier(attack, 3, 2);
    }
  }
  // DragonsMaw, Steelworker, Transistor
  // Transistor was still 1.5 boost in Gen8
  if ((attacker.has_ability(Ability::DRAGONS_MAW) && move.type == Type::DRAGON) ||
      (attacker.has_ability(Ability::STEELWORKER) && move.type == Type::STEEL) ||
      (attacker.has_ability(Ability::TRANSISTOR) && move.type == Type::ELECTRIC)) {
    attack = applyModifier(attack, 3, 2);
  }
  // // FlashFire
  // if(attacker.flashfire && attacker.has_ability(Ability::FLASH_FIRE) && move.type == Type::FIRE)
  // {
  //   attack = applyModifier(attack, 3, 2);
  // }
  // Stakeout
  if (attacker.has_ability(Ability::STAKEOUT) && !defender.activeTurns) {
    attack *= 2;
  }
  // PRIORITY 1
  // APPLIES TO ONLY ATK
  // // GorillaTactics, ChoiceBand
  // if(attacker.has_ability(Ability::GORILLA_TACTICS) || attacker.has_item(Item::CHOICE_BAND)) {
  //   attack = applyModifier(attack, 3, 2);
  // }
  // ThickClub
  if ((attacker.species == Species::CUBONE || attacker.species == Species::MAROWAK) &&
      attacker.has_item(Item::THICK_CLUB)) {
    attack *= 2;
  }
  // APPLIES TO BOTH
  // LightBall
  if (attacker.species == Species::PIKACHU && attacker.has_item(Item::LIGHT_BALL)) {
    attack *= 2;
  }
  return attack;
}
// NB: all use chainModify...what's the difference?
int BattleState::applyOnModifyDef(int defense, Pokemon const &defender) {
  // PRIORITY 10: Snowscape only (not Hail)
  // PRIORITY 6
  // FurCoat
  if (defender.has_ability(Ability::FUR_COAT)) {
    defense *= 2;
  }
  // GrassPelt
  if (defender.has_ability(Ability::GRASS_PELT) && terrain == Terrain::GRASSY) {
    defense = applyModifier(defense, 3, 2);
  }
  // MarvelScale
  if (defender.has_ability(Ability::MARVEL_SCALE) && defender.status != Status::NO_STATUS) {
    defense = applyModifier(defense, 3, 2);
  }
  // PRIORITY 2
  // // Eviolite
  // if(defender.has_item(Item::EVIOLITE) && !defender.fully_evolved()) {
  //   defense = applyModifier(defense, 3, 2);
  // }
  // // MetalPowder
  // if(defender.has_item(Item::METAL_POWDER) && defender.species == Species::DITTO &&
  // !defender.transformed) {
  //   defense *= 2;
  // }
  return defense;
}
int BattleState::applyOnModifySpA(int attack, Pokemon const &attacker, Pokemon const &defender,
                                  Move const &move) {
  // PRIORITY 5
  // APPLIES TO ONLY SPA
  // Plus and Minus aren't applicable in Singles
  // SolarPower
  if (attacker.has_ability(Ability::SOLAR_POWER) && isSunny()) {
    attack = applyModifier(attack, 3, 2);
  }
  // WaterBubble
  if (attacker.has_ability(Ability::WATER_BUBBLE) && move.type == Type::WATER) {
    attack *= 2;
  }
  // APPLIES TO BOTH
  // Defeatist
  if (attacker.has_ability(Ability::DEFEATIST) && attacker.current_hp * 2 <= attacker.stats.hp) {
    attack /= 2;
  }
  // Blaze, Overgrow, Torrent, Swarm
  if (attacker.current_hp * 3 <= attacker.stats.hp) {
    if ((attacker.has_ability(Ability::BLAZE) && move.type == Type::FIRE) ||
        (attacker.has_ability(Ability::OVERGROW) && move.type == Type::GRASS) ||
        (attacker.has_ability(Ability::TORRENT) && move.type == Type::WATER) ||
        (attacker.has_ability(Ability::SWARM) && move.type == Type::BUG)) {
      attack = applyModifier(attack, 3, 2);
    }
  }
  // DragonsMaw, Steelworker, Transistor
  // Transistor was still 1.5 boost in Gen8
  if ((attacker.has_ability(Ability::DRAGONS_MAW) && move.type == Type::DRAGON) ||
      (attacker.has_ability(Ability::STEELWORKER) && move.type == Type::STEEL) ||
      (attacker.has_ability(Ability::TRANSISTOR) && move.type == Type::ELECTRIC)) {
    attack = applyModifier(attack, 3, 2);
  }
  // // FlashFire
  // if(attacker.flashfire && attacker.has_ability(Ability::FLASH_FIRE) && move.type == Type::FIRE)
  // {
  //   attack = applyModifier(attack, 3, 2);
  // }
  // Stakeout
  if (attacker.has_ability(Ability::STAKEOUT) && !defender.activeTurns) {
    attack *= 2;
  }
  // PRIORITY 1
  // APPLIES TO ONLY SPA
  // // ChoiceSpecs
  // if(attacker.has_item(Item::CHOICE_SPECS)) {
  //   attack = applyModifier(attack, 3, 2);
  // }
  // DeepSeaTooth
  if (attacker.species == Species::CLAMPERL && attacker.has_item(Item::DEEP_SEA_TOOTH)) {
    attack *= 2;
  }
  // APPLIES TO BOTH
  // LightBall
  if (attacker.species == Species::PIKACHU && attacker.has_item(Item::LIGHT_BALL)) {
    attack *= 2;
  }
  return attack;
}
int BattleState::applyOnModifySpD(int defense, Pokemon const &defender) {
  // PRIORITY 10
  // Sandstorm
  if (weather == Weather::SANDSTORM && defender.has_type(Type::ROCK)) {
    defense = applyModifier(defense, 3, 2);
  }
  // PRIORITY 2
  // // Eviolite
  // if(defender.has_item(Item::EVIOLITE) && !defender.fully_evolved()) {
  //   defense = applyModifier(defense, 3, 2);
  // }
  // DeepSeaScale
  if (defender.species == Species::CLAMPERL && defender.has_item(Item::DEEP_SEA_SCALE)) {
    defense *= 2;
  }
  // PRIORITY 1
  // AssaultVest
  if (defender.has_item(Item::ASSAULT_VEST)) {
    defense = applyModifier(defense, 3, 2);
  }
  return defense;
}
// TODO: onStart callback
void BattleState::applyOnStart(Pokemon &pokemon) {}
// Callback when flinched
void BattleState::applyOnFlinch(Pokemon &pokemon) {
  if (pokemon.ability == Ability::STEADFAST) {
    pokemon.boostStat(ModifierId::SPEED, 1);
  }
}
// In PS terms: For checking onBeforeMove returning false.
// Returns if the move fails for a StompingTantrum-activating reason.
bool BattleState::applyOnBeforeMoveST(Pokemon &pokemon) {
  // PRIORITY 8
  if (pokemon.flinch) {
    applyOnFlinch(pokemon);
    pokemon.flinch = false;
    return true;
  }
  // TODO: remaining conditions and stuff
  return false;
}
// Callback potentially causing non-StompingTantrum move failures.
// In PS terms: For checking onBeforeMove returning null.
bool BattleState::applyOnBeforeMove(Pokemon &pokemon) {
  // TODO: remaining conditions and stuff
  // (pre-move stops like par, slp, choice locks, etc.)
  // Bide and Focus Punch special case stops PP deduction
  // PRIORITY 3
  if (pokemon.confusion) {
    pokemon.confusion--;
    if (pokemon.confusion && math::randomChance(33, 100)) {
      // dmg = getConfusionDamage(pokemon, 40):
      const int bp = 40; // basePower
      int attack =
          pokemon.calculateStat(ModifierId::ATTACK, pokemon.boosts[ModifierId::ATTACK], pokemon);
      int defense =
          pokemon.calculateStat(ModifierId::DEFENSE, pokemon.boosts[ModifierId::DEFENSE], pokemon);
      int dmg = (((2 * pokemon.lvl / 5 + 2) * bp * attack) / defense) / 50 + 2;
      // Damage is 16-bit context in self-hit confusion damage
      dmg %= 1 << 16;
      DMGCalcOptions opt = {};
      dmg = generateAndApplyDmgRoll(dmg, opt);
      dmg = std::max(1, dmg);
      // std::cout << "confusion damage: " << dmg << std::endl;
      spreadDamage({dmg}, pokemon, pokemon, EffectKind::CONFUSION, MoveId::NONE);
      return false;
    }
  }
  pokemon.moveThisTurnFailed = false;
  return true;
}
bool BattleState::applyOnLockMove(Pokemon &user) {
  if (user.lockedMove)
    return true;
  if (user.twoTurnMove)
    return true;
  // if(user.mustRecharge) return true;
  // if(user.biding) return true;
  // if(user.iceball) return true;
  // if(user.rollout) return true;
  // if(user.uproar) return true;
  return false;
}
// Certain volatile removal
void BattleState::applyOnAfterMove(Pokemon &target, Pokemon &user, MoveInstance &moveInst) {
  switch (moveInst.id) {
  case MoveId::BEAKBLAST: {
    user.beakBlasting = false;
    break;
  }
  case MoveId::OUTRAGE:
  case MoveId::PETALDANCE:
  case MoveId::RAGINGFURY:
  case MoveId::THRASH: {
    if (user.lockedMove == 1) {
      user.lockedMove = 0;
    }
    break;
  }
  case MoveId::SPARKLINGARIA: {
    Pokemon &activeFoe = teams[1 - user.side].pkmn[teams[1 - user.side].activeInd];
    if (activeFoe.sparklingAria) {
      activeFoe.sparklingAria = false;
      if (activeFoe.status == Status::BURN && !user.fainted) {
        activeFoe.status = Status::NO_STATUS;
      }
    }
    break;
  }
  case MoveId::SPITUP: {
    user.stockpileTurns = 0;
  }
    // case MoveId::ICEBALL:
    // case MoveId::ROLLOUT:
    // case MoveId::MINDBLOWN:
    // case MoveId::STEELBEAM;
  default:
    break;
  }
  if (user.charging && moveInst.id != MoveId::CHARGE) {
    user.charging = false;
  }
}
// Given a two-turn move, returns whether or not to take the charging turn.
bool BattleState::applyOnChargeMove(Pokemon &user) {
  if (user.has_item(Item::POWER_HERB) && user.useItem(false, false)) {
    return false;
  }
  return true;
}
// Some weather-related effects
bool BattleState::applyOnTryMove(Pokemon &target, Pokemon &user, MoveInstance &moveInst) {
  Move &move = moveInst.moveData;
  // PRIORITY 1
  // Intense weather cause opposite-type moves to fail
  if (weather == Weather::PRIMORDIAL_SEA && move.type == Type::FIRE &&
      move.category != MoveCategory::STATUS) {
    return false;
  }
  if (weather == Weather::DESOLATE_LAND && move.type == Type::WATER &&
      move.category != MoveCategory::STATUS) {
    return false;
  }
  // PRIORITY 0
  // Two-turn (semi-invuln or charge) moves:
  // - Special Dive+GulpMissile case
  static const std::vector<MoveId> twoTurnMoves = {
      MoveId::BOUNCE,     MoveId::DIG,          MoveId::DIVE,      MoveId::ELECTROSHOT,
      MoveId::FLY,        MoveId::FREEZESHOCK,  MoveId::GEOMANCY,  MoveId::ICEBURN,
      MoveId::METEORBEAM, MoveId::PHANTOMFORCE, MoveId::RAZORWIND, MoveId::SHADOWFORCE,
      MoveId::SKULLBASH,  MoveId::SKYATTACK,    MoveId::SOLARBEAM, MoveId::SOLARBLADE};
  if (std::find(twoTurnMoves.begin(), twoTurnMoves.end(), moveInst.id) != twoTurnMoves.end()) {
    // // Check if second turn of usage
    if (user.twoTurnMove) {
      return true;
    }
    // Initial-use boosts
    if (moveInst.id == MoveId::ELECTROSHOT || moveInst.id == MoveId::METEORBEAM) {
      user.boostStat(ModifierId::SPATT, 1);
    }
    if (moveInst.id == MoveId::SKULLBASH) {
      user.boostStat(ModifierId::DEFENSE, 1);
    }
    // Weather-based insta-charges
    if (moveInst.id == MoveId::ELECTROSHOT && isRainy()) {
      return true;
    }
    if ((moveInst.id == MoveId::SOLARBEAM || moveInst.id == MoveId::SOLARBLADE) && isSunny()) {
      return true;
    }
    // Check if otherwise insta-charge
    if (!applyOnChargeMove(user)) {
      return true;
    }
    // Initiate first turn of usage -- cancel rest of moveSteps
    // Q: if the target changes out, does the move fail? Or is the slot targeted?
    user.twoTurnMove = true;
    return false;
  }
  // Type-losing moves (BurnUp, DoubleShock)
  // PollenPuff
  // ShellTrap
  // PRIORITY -1
  // Powder
  // PRIORITY -2
  // Metronome item: complicated
  return true;
}
// Effects directly changing damage amount
int BattleState::applyOnDamage(int damage, Pokemon &target, Pokemon &source, EffectKind effectKind,
                               MoveId effectMove) {
  // PRIORITY 1
  // Disguise
  // IceFace
  // PoisonHeal
  if (target.has_ability(Ability::POISON_HEAL) &&
      (effectKind == EffectKind::POISON || effectKind == EffectKind::TOXIC)) {
    target.heal(target.stats.hp / 8, EffectKind::NO_EFFECT);
    return 0;
  }
  // PRIORITY 0
  // Berserk (AngerShell is Gen 9)
  // Gluttony?
  // Heatproof
  if (target.has_ability(Ability::HEATPROOF) && effectKind == EffectKind::BURN) {
    damage /= 2;
  }
  // MagicGuard
  if (target.has_ability(Ability::MAGIC_GUARD) && effectKind != EffectKind::MOVE) {
    return 0;
  }
  // RockHead
  if (target.has_ability(Ability::ROCK_HEAD) && effectKind == EffectKind::RECOIL) {
    if (effectMove != MoveId::STRUGGLE) {
      return 0;
    }
  }
  // PRIORITY -10
  // Endure
  if (target.endure && effectKind == EffectKind::MOVE) {
    if (damage >= target.current_hp) {
      return target.current_hp - 1;
    }
  }
  // PRIORITY -20
  // FalseSwipe, HoldBack
  if (effectMove == MoveId::FALSESWIPE || effectMove == MoveId::HOLDBACK) {
    if (damage >= target.current_hp) {
      return target.current_hp - 1;
    }
  }
  // Sturdy (PRIORITY -30), FocusSash (PRIORITY -40)
  if ((target.has_ability(Ability::STURDY) || target.has_item(Item::FOCUS_SASH)) &&
      effectKind == EffectKind::MOVE) {
    if (target.current_hp == target.stats.hp && damage >= target.current_hp) {
      // FocusSash is low onDamagePriority
      if (target.has_ability(Ability::STURDY) || target.useItem(false, false)) {
        return target.current_hp - 1;
      }
    }
  }
  // FocusBand (PRIORITY -40)
  if (target.has_item(Item::FOCUS_BAND) && effectKind == EffectKind::MOVE) {
    if (damage >= target.current_hp && math::randomChance(1, 10)) {
      return target.current_hp - 1;
    }
  }
  // PRIORITY -101
  // Bide: very complicated
  return damage;
}
// Crash damage and SkyDrop failure
// TODO: failure from no target shouldn't deal crash damage
void BattleState::applyOnMoveFail(Pokemon &target, Pokemon &pokemon, MoveInstance &moveInst) {
  static const std::vector<MoveId> crashDmg = {MoveId::AXEKICK, MoveId::HIGHJUMPKICK,
                                               MoveId::JUMPKICK, MoveId::SUPERCELLSLAM};
  if (std::find(crashDmg.begin(), crashDmg.end(), moveInst.id) != crashDmg.end()) {
    // Directly deal damage to itself
    spreadDamage(DamageResultState(pokemon.stats.hp / 2), pokemon, pokemon,
                 EffectKind::CRASH_DAMAGE, MoveId::NONE);
  }
  // TODO: SkyDrop
}
// Check pre-conditions of using the particular move.
bool BattleState::applyOnTry(Pokemon &user, Pokemon &target, MoveInstance &moveInst) {
  switch (moveInst.id) {
  case MoveId::AURAWHEEL: {
    return user.species == Species::MORPEKO;
  }
  case MoveId::AURORAVEIL: {
    return weather == Weather::HAIL;
  }
  case MoveId::CLANGOROUSSOUL: {
    return user.stats.hp > 1 && user.current_hp > user.stats.hp * 33 / 100;
  }
  case MoveId::DARKVOID: {
    return user.species == Species::DARKRAI;
  }
  case MoveId::HYPERSPACEFURY: {
    return user.name == PokeName::HOOPA_UNBOUND;
  }
  case MoveId::MAGNETRISE: {
    return !target.smackeddown && !target.ingrained && !gravity;
  }
  case MoveId::NORETREAT: {
    // TODO: avoid applying this volatile if already trapped
    return !user.noretreat;
  }
  case MoveId::POLTERGEIST: {
    return !target.has_item(Item::NO_ITEM);
  }
  case MoveId::SKYDROP: {
    return !target.fainted;
  }
  // case MoveId::CRAFTYSHIELD: // Why is this onTry() a thing?
  // case MoveId::ECHOEDVOICE: {
  //   addPseudoWeather(echoedvoice);
  // }
  case MoveId::REST: {
    if (user.status == Status::SLEEP || user.has_ability(Ability::COMATOSE) ||
        user.has_ability(Ability::INSOMNIA) || user.has_ability(Ability::VITAL_SPIRIT))
      return false;
    if (user.current_hp == user.stats.hp)
      return false;
    break;
  }
  case MoveId::SLEEPTALK:
  case MoveId::SNORE: {
    return user.status == Status::SLEEP || user.has_ability(Ability::COMATOSE);
  }
  case MoveId::SPLASH:
  case MoveId::TELEKINESIS: {
    return !gravity;
  }
  case MoveId::STEELROLLER: {
    return terrain != Terrain::NO_TERRAIN;
  }
  case MoveId::STUFFCHEEKS: {
    return isBerry(user.item);
  }
  case MoveId::FAKEOUT:
  case MoveId::FIRSTIMPRESSION: {
    return user.activeMoveActions <= 1;
  }
  case MoveId::MATBLOCK: {
    return user.activeMoveActions <= 1 && queueWillAct;
  }
  case MoveId::QUICKGUARD:
  case MoveId::WIDEGUARD: {
    return queueWillAct;
  }
  case MoveId::LASTRESORT: {
    // Must have at least 2 moves and must have LastResort
    bool hasLastResort = false;
    int numMoves = 0;
    for (int moveSlot = 0; moveSlot < 4; moveSlot++) {
      if (user.moves[moveSlot].id != MoveId::NONE) {
        numMoves++;
        if (user.moves[moveSlot].id == MoveId::LASTRESORT) {
          hasLastResort = true;
        }
      }
    }
    return numMoves > 1 && hasLastResort;
  }
  // case MoveId::TELEPORT:
  // case MoveId::SUCKERPUNCH:
  // case MoveId::SPITUP:
  // case MoveId::SWALLOW:
  // case MoveId::STOCKPILE:
  // case MoveId::ROUND: // Prioritize other Round moves
  // case MoveId::DOOMDESIRE: // future-move setup
  // case MoveId::FUTURESIGHT:
  // case MoveId::COUNTER:
  // case MoveId::MIRROCOAT:
  // case MoveId::METALBURST:
  // These fail in single battles
  case MoveId::RAGEPOWDER:
  case MoveId::FOLLOWME: {
    return false;
  }
  default:
    break;
  }
  return true;
}
bool BattleState::applyOnStallMove(Pokemon &user) {
  if (math::randomChance(1, user.stallCounter)) {
    // Reset stall volatile
    user.stallCounter = 1;
    user.stallTurns = 0;
    return false;
  }
  return true;
}
// Checks both user's ability and move used for state-changing move pre-conditions
// Not sure if I should check ability or move first...
bool BattleState::applyOnPrepareHit(Pokemon &user, Pokemon &target, MoveInstance &moveInst) {
  switch (user.ability) {
  case Ability::LIBERO:
  case Ability::PROTEAN: {
    if (!moveInst.hasBounced && !moveInst.isFuture() && !moveInst.isCalling()) {
      // TODO: Snatch sourceeffect check
      Type type = moveInst.moveData.type;
      if (type != Type::NO_TYPE && !user.isOnlyType(type)) {
        user.set_type({type});
      }
    }
    break;
  }
  case Ability::PARENTAL_BOND: {
    Move &move = moveInst.moveData;
    if (move.category != MoveCategory::STATUS && !moveInst.isMultiHit() &&
        !moveInst.isNoParentalBond() && !moveInst.isCharge() && !moveInst.isFuture()) {
      // In singles, there are no multi-target moves, so spreadHit is never true.
      moveInst.parentalBond = true;
    }
    break;
  }
  default:
    break;
  }
  switch (moveInst.id) {
  // case MoveId::ALLYSWITCH:
  case MoveId::BANEFULBUNKER:
  case MoveId::BURNINGBULWARK:
  case MoveId::DETECT:
  case MoveId::ENDURE:
  case MoveId::KINGSSHIELD:
  case MoveId::OBSTRUCT:
  case MoveId::PROTECT:
  case MoveId::SPIKYSHIELD: {
    return queueWillAct && applyOnStallMove(user);
  }
  // case MoveId::DESTINYBOND: { // TODO
  //   return !user.removeVolatile(destinybond);
  // }
  // case MoveId::SHELLSIDEARM: // Just adds logging, so skip
  // case MoveId::FLING:
  // case MoveId::NATURALGIFT:
  // Skip Pledges
  // case MoveId::FIREPLEDGE:
  // case MoveId::GRASSPLEDGE:
  // case MoveId::WATERPLEDGE:
  default:
    break;
  }
  return true;
}
// Numerical accuracy adjustments
int BattleState::applyOnModifyAccuracy(Pokemon const &target, Pokemon const &pokemon,
                                       MoveInstance &moveInst, int acc) {
  if (target.has_item(Item::BRIGHT_POWDER) || target.has_item(Item::LAX_INCENSE)) {
    acc = applyModifier(acc, 3686, 4096);
  }
  if ((weather == Weather::SANDSTORM && target.ability == Ability::SAND_VEIL) ||
      (weather == Weather::HAIL && target.ability == Ability::SNOW_CLOAK)) {
    acc = applyModifier(acc, 3277, 4096);
  }
  if (target.confusion > 0 && target.ability == Ability::TANGLED_FEET) {
    acc /= 2;
  }
  // Bulbapedia says it only *lowers* the accuracy? Not worth testing atm.
  if (target.ability == Ability::WONDER_SKIN &&
      moveInst.moveData.category == MoveCategory::STATUS) {
    acc = 50;
  }
  return acc;
}
// Certain conditions on the target result in the move always-hitting
bool BattleState::applyOnAccuracy(Pokemon const &target, Pokemon const &pokemon,
                                  MoveInstance &moveInst) {
  // Skip Glaive Rush (Gen 9)
  // Minimize interaction: certain moves *always* hit
  static const std::vector<MoveId> boostedMoves = {
      MoveId::STOMP,      MoveId::STEAMROLLER, MoveId::BODYSLAM,  MoveId::FLYINGPRESS,
      MoveId::DRAGONRUSH, MoveId::HEATCRASH,   MoveId::HEAVYSLAM, MoveId::SUPERCELLSLAM};
  if (target.minimized &&
      std::find(boostedMoves.begin(), boostedMoves.end(), moveInst.id) != boostedMoves.end()) {
    return true;
  }
  // Telekinesis: all moves *always* hit
  if (target.telekinesised && !moveInst.isOHKO()) {
    return true;
  }
  return false;
}
// Returns false if target is in semi-invuln state that blocks moveInst.
// Called by trySpreadMoveHit() in hitStepInvulnerabilityEvent()
bool BattleState::applyOnInvulnerability(Pokemon &target, MoveInstance &moveInst) {
  static const std::vector<MoveId> hitsInAir = {
      MoveId::GUST,      MoveId::TWISTER,   MoveId::SKYUPPERCUT,    MoveId::THUNDER,
      MoveId::HURRICANE, MoveId::SMACKDOWN, MoveId::THOUSANDARROWS,
  };
  static const std::vector<MoveId> hitsUndeground = {
      MoveId::EARTHQUAKE,
      MoveId::MAGNITUDE,
  };
  static const std::vector<MoveId> hitsUndewater = {
      MoveId::SURF,
      MoveId::WHIRLPOOL,
  };
  if (target.inAir) {
    if (std::find(hitsInAir.begin(), hitsInAir.end(), moveInst.id) == hitsInAir.end()) {
      return false;
    }
  }
  if (target.underground) {
    if (std::find(hitsUndeground.begin(), hitsUndeground.end(), moveInst.id) ==
        hitsUndeground.end()) {
      return false;
    }
  }
  if (target.underwater) {
    if (std::find(hitsUndewater.begin(), hitsUndewater.end(), moveInst.id) == hitsUndewater.end()) {
      return false;
    }
  }
  if (target.inShadow)
    return false;
  return true;
}
// Note distinguishment from onAfterMoveSecondary*Self*
void BattleState::applyOnAfterMoveSecondary(Pokemon &target, Pokemon &user,
                                            MoveInstance &moveInst) {
  Move &move = moveInst.moveData;
  // TODO: Berserk
  // ColorChange
  if (target.has_ability(Ability::COLOR_CHANGE) && target.current_hp && target.isActive &&
      move.category != MoveCategory::STATUS && move.type != Type::NO_TYPE &&
      !target.has_type(move.type)) {
    target.set_type({move.type});
    // TODO: Arceus special case: potential fail in set_type
  }
  // TODO: Pickpocket
  // // TODO: Thaw
  // if (target.status == Status::FREEZE && moveInst.thawsTarget) {
  //   target.status = Status::NO_STATUS;
  // }
  // TODO: EjectButton, RedCard
  // // TODO: KeeBerry, MarangaBerry
  // if (move.category == MoveCategory::PHYSICAL) {
  //   if (move.id == MoveId::PRESENT && moveInst.heal)
  //     return;
  //   target.eatItem();
  // }
}
void BattleState::applyOnEmergencyExit(int prevHP, Pokemon &pokemon) {
  if (pokemon.current_hp <= pokemon.stats.hp / 2 && prevHP > pokemon.stats.hp / 2) {
    if (pokemon.has_ability(Ability::EMERGENCY_EXIT) || pokemon.has_ability(Ability::WIMP_OUT)) {
      if (teams[pokemon.side].pokemonLeft && !pokemon.forceSwitchFlag && !pokemon.switchFlag) {
        for (int side = 0; side < 2; side++) {
          teams[side].pkmn[teams[side].activeInd].switchFlag = false;
        }
        pokemon.switchFlag = true;
      }
    }
  }
}
void BattleState::applyOnEnd(Pokemon &pokemon) {
  // // Airlock, CloudNine
  // pokemon.abilityState.ending = true;
  // applyOnWeatherChange(AIRLOCK);
  // // DeltaStream, DesolateLand, PrimordialSea
  // if (field.weatherState.source !== pokemon) return;
  // for (Pokemon *target : getAllActivePokemon()) {
  //   if (target is pokemon) continue;
  //   if (target.hasAbility('deltastream')) {
  //     field.weatherState.source = target;
  //     return;
  //   }
  // }
  // clearWeather();
  // // FlashFire, SlowStart, Unburden
  // pokemon.removeVolatile('flashfire');
  // // GorillaTactics
  // pokemon.abilityState.choicelock = NONE;
  // Clear Illusion
  // NeutralizingGas
  // // Unnerve
  // effectState.unnerved = false;
  // ZenMode: delete volatile and forme change
  // PowerShift, PowerTrick
}
// Only Unburden (no Symbiosis in singles)
void BattleState::applyOnAfterUseItem(Pokemon &target) {
  if (target.has_ability(Ability::UNBURDEN)) {
    // Why check target == effectState.target?
    target.unburden = true;
  }
}
// TODO: finish implementing
bool BattleState::applyOnTryHit(Pokemon &target, Pokemon &user, MoveInstance &moveInst) {
  // std::cout << "side " << user.side << " using move " << (int)moveInst.id << std::endl;
  return true;
}
// Returns false if the secondary effect is blocked (by ShieldDust/CovertCloak)
bool BattleState::applyOnModifySecondaries(Pokemon &target, MoveInstance &moveInst,
                                           Secondary secondaryKind) {
  if (target.has_ability(Ability::SHIELD_DUST) || target.has_item(Item::COVERT_CLOAK)) {
    return moveInst.id == MoveId::SPARKLINGARIA || secondaryKind == Secondary::SELFBOOST;
  }
  return true;
}
// Returns whether the target can be dragged out
bool BattleState::applyOnDragOut(Pokemon &target) {
  if (target.has_ability(Ability::SUCTION_CUPS))
    return false;
  if (target.ingrained)
    return false;
  return true;
}
//
void BattleState::applyOnDamagingHit(int damage, Pokemon &target, Pokemon &user,
                                     MoveInstance &moveInst) {
  Move const &move = moveInst.moveData;
  // Thawing
  if (target.status == Status::FREEZE && move.type == Type::FIRE &&
      move.category != MoveCategory::STATUS) {
    target.status = Status::NO_STATUS;
  }
  // TODO: Counter
  // TODO: MirrorCoat
  // ------ ITEMS ------
  switch (target.item) {
  case Item::ABSORB_BULB:
  case Item::LUMINOUS_MOSS: {
    if (move.type == Type::WATER) {
      target.useItem(false, false);
    }
    break;
  }
  case Item::CELL_BATTERY: {
    if (move.type == Type::ELECTRIC) {
      target.useItem(false, false);
    }
    break;
  }
  case Item::SNOWBALL: {
    if (move.type == Type::ICE) {
      target.useItem(false, false);
    }
    break;
  }
  case Item::ROCKY_HELMET: {
    if (moveInst.makesContact(user)) {
      spreadDamage({user.stats.hp / 6}, user, target, EffectKind::ROCKY_HELMET, move.id);
    }
    break;
  }
  // TODO
  case Item::WEAKNESS_POLICY: {
    // Conditions:
    // - Move damage calculation isn't e.g. level/fixed (so can be super-eff)
    // - Move typing is super effective
    // then useItem()
    break;
  }
  case Item::JABOCA_BERRY: {
    if (move.category == MoveCategory::PHYSICAL && user.current_hp && user.isActive &&
        !user.has_ability(Ability::MAGIC_GUARD)) {
      if (target.useItem(true, false)) {
        int dmg = user.stats.hp / (target.has_ability(Ability::RIPEN) ? 4 : 8);
        spreadDamage({dmg}, user, target, EffectKind::BERRY, MoveId::NONE);
      }
    }
    break;
  }
  case Item::ROWAP_BERRY: {
    if (move.category == MoveCategory::SPECIAL && user.current_hp && user.isActive &&
        !user.has_ability(Ability::MAGIC_GUARD)) {
      if (target.useItem(true, false)) {
        int dmg = user.stats.hp / (target.has_ability(Ability::RIPEN) ? 4 : 8);
        spreadDamage({dmg}, user, target, EffectKind::BERRY, MoveId::NONE);
      }
    }
    break;
  }
    // case Item::AIR_BALLOON:
  default:
    break;
  }
  // ------ ABILITIES ------
  // TODO
  switch (target.ability) {
  default:
    break;
  }
}
// TODO: Most of the callbacks!
bool BattleState::applyOnHitMain(Pokemon &target, Pokemon &user, MoveInstance &moveInst) {
  switch (moveInst.id) {
  case MoveId::PLUCK: {
    Item item = target.item;
    if (user.current_hp && isBerry(item) && target.takeItem(user) != std::nullopt) {
      user.applyOnEat(item);
      user.applyOnEatItem(item);
      // LeppaBerry staleness?
      // Set ateBerry?
    }
    break;
  }
  default:
    break;
  }
  return true;
}
// TODO: A few miscellaneous effects that are applied after a move hits.
void BattleState::applyOnAfterHit(Pokemon &target, Pokemon &user, MoveInstance &moveInst) {
  switch (moveInst.id) {
  case MoveId::ICESPINNER: {
    // Clear terrain
    break;
  }
  case MoveId::RAPIDSPIN: {
    // Clears lots of stuff
    break;
  }
  case MoveId::CEASELESSEDGE: {
    // Leaves spikes
    break;
  }
  case MoveId::STONEAXE: {
    // Leaves stealth rock
    break;
  }
  case MoveId::KNOCKOFF: {
    // Knocks off item
    break;
  }
  case MoveId::THIEF:
  case MoveId::COVET: {
    // Steals item
    break;
  }
    // case MoveId::MORTALSPIN: // Gen9
  default:
    break;
  }
}
// Run onUpdate() on a single Pokemon.
void BattleState::applyOnUpdate(Pokemon &target) {
  switch (target.ability) {
  case Ability::IMMUNITY:
  case Ability::PASTEL_VEIL: {
    if (target.status == Status::POISON || target.status == Status::TOXIC) {
      target.status = Status::NO_STATUS;
    }
    break;
  }
  case Ability::INSOMNIA:
  case Ability::VITAL_SPIRIT: {
    if (target.status == Status::SLEEP) {
      target.status = Status::NO_STATUS;
    }
    break;
  }
  case Ability::LIMBER: {
    if (target.status == Status::PARALYSIS) {
      target.status = Status::NO_STATUS;
    }
    break;
  }
  case Ability::MAGMA_ARMOR: {
    if (target.status == Status::FREEZE) {
      target.status = Status::NO_STATUS;
    }
    break;
  }
  case Ability::THERMAL_EXCHANGE:
  case Ability::WATER_BUBBLE:
  case Ability::WATER_VEIL: {
    if (target.status == Status::BURN) {
      target.status = Status::NO_STATUS;
    }
    break;
  }
  case Ability::OBLIVIOUS: {
    if (target.attracted) {
      target.attracted = false;
    }
    if (target.taunted) {
      target.taunted = 0;
    }
    break;
  }
  case Ability::OWN_TEMPO: {
    if (target.confusion) {
      target.confusion = 0;
    }
    break;
  }
  case Ability::TRACE: {
    Team &otherTeam = teams[1 - target.side];
    Pokemon &activeFoe = otherTeam.pkmn[otherTeam.activeInd];
    if (!noTrace(activeFoe.ability) && activeFoe.ability != Ability::NO_ABILITY) {
      target.set_ability(activeFoe.ability);
    }
    break;
  }
    // case Ability::DISGUISE:
    // case Ability::ICEFACE:
  default:
    break;
  }
  switch (target.item) {
  case Item::AGUAV_BERRY:
  case Item::APICOT_BERRY:
  case Item::FIGY_BERRY:
  case Item::GANLON_BERRY:
  case Item::IAPAPA_BERRY:
  case Item::LANSAT_BERRY:
  case Item::LIECHI_BERRY:
  case Item::MAGO_BERRY:
  case Item::PETAYA_BERRY:
  case Item::SALAC_BERRY:
  case Item::STARF_BERRY:
  case Item::WIKI_BERRY: {
    if (target.current_hp <= target.stats.hp / 4 ||
        (target.current_hp <= target.stats.hp / 2 && target.has_ability(Ability::GLUTTONY)))
      target.useItem(true, false);
    break;
  }
  case Item::ASPEAR_BERRY: {
    if (target.status == Status::FREEZE)
      target.useItem(true, false);
    break;
  }
  case Item::RAWST_BERRY: {
    if (target.status == Status::BURN)
      target.useItem(true, false);
    break;
  }
  case Item::CHERI_BERRY: {
    if (target.status == Status::PARALYSIS)
      target.useItem(true, false);
    break;
  }
  case Item::CHESTO_BERRY: {
    if (target.status == Status::SLEEP)
      target.useItem(true, false);
    break;
  }
  case Item::PECHA_BERRY: {
    if (target.status == Status::POISON || target.status == Status::TOXIC)
      target.useItem(true, false);
    break;
  }
  case Item::PERSIM_BERRY: {
    if (target.confusion)
      target.useItem(true, false);
    break;
  }
  case Item::LUM_BERRY: {
    if (target.status != Status::NO_STATUS || target.confusion)
      target.useItem(true, false);
    break;
  }
  case Item::LEPPA_BERRY: {
    if (!target.current_hp)
      return;
    for (int i = 0; i < 4; i++) {
      if (target.moves[i].id != MoveId::NONE && target.moves[i].pp == 0) {
        target.useItem(true, false);
        break;
      }
    }
    break;
  }
  case Item::ORAN_BERRY:
  case Item::SITRUS_BERRY: {
    if (target.current_hp <= target.stats.hp / 2) {
      // std::cout << "hp before: " << target.current_hp << std::endl;
      target.useItem(true, false);
      // std::cout << "hp after: " << target.current_hp << std::endl;
    }
    break;
  }
  case Item::BERRY_JUICE: {
    if (target.current_hp <= target.stats.hp / 2 &&
        target.applyOnTryHeal(20, EffectKind::BERRY_JUICE) && target.useItem(false, false))
      target.applyHeal(20);
    break;
  }
    // case Item::MENTAL_HERB:
    // case Item::WHITE_HERB:
    // case Item::UTILITY_UMBRELLA:
  default:
    break;
  }
  // If an effect source is no longer on the field, remove the effect
  // case attract:
  // Fling
  if (target.flungThisTurn) {
    Item oldItem = target.item;
    target.set_item(Item::NO_ITEM);
    target.lastItem = oldItem;
    target.usedItemThisTurn = true;
    applyOnAfterUseItem(target);
    target.flungThisTurn = false;
  }
  // MegaGengar-Telekinesis interaction
  if (target.name == PokeName::MEGA_GENGAR) {
    target.telekinesised = false;
  }
}
// VERY IMPORTANT TODO
void BattleState::applyOnResidual(Pokemon &target) {
  // TODO
}
// Run onUpdate() or onResidual() on each active Pokemon in speed order.
void BattleState::applyOnEach(EachEventKind event) {
  std::vector<Pokemon *> activePokemon;
  for (int side = 0; side < 2; side++) {
    Pokemon &activePkmn = teams[side].pkmn[teams[side].activeInd];
    if (!activePkmn.fainted) {
      activePokemon.push_back(&activePkmn);
    }
  }
  // Sort in decreasing speed order
  std::sort(activePokemon.begin(), activePokemon.end(),
            [](Pokemon *a, Pokemon *b) { return b->effectiveSpeed - a->effectiveSpeed; });
  // Speed tie case
  if (activePokemon.size() == 2 &&
      activePokemon[0]->effectiveSpeed == activePokemon[1]->effectiveSpeed &&
      math::randomChance(1, 2)) {
    std::reverse(activePokemon.begin(), activePokemon.end());
  }
  for (Pokemon *pkmnPtr : activePokemon) {
    switch (event) {
    case EachEventKind::UPDATE: {
      applyOnUpdate(*pkmnPtr);
      break;
    }
    case EachEventKind::RESIDUAL: {
      applyOnResidual(*pkmnPtr);
      break;
    }
    }
  }
}
// TODO: finish implementing
bool BattleState::applyOnRestart(VolatileId vol, Pokemon &target, Pokemon &source) {
  switch (vol) {
  case VolatileId::STALL: {
    if (source.stallCounter < 729) {
      source.stallCounter *= 3;
    }
    source.stallTurns = 2;
  }
  // case VolatileId::SMACKDOWN: {
  //   if(target.removeVolatile(VolatileId::FLY) || target.removeVolatile(VolatileId::BOUNCE)) {
  //     queue.cancelMove(target);
  //     target.removeVolatile(VolatileId::TWOTURN_MOVE);
  //   }
  // }
  // Purely a cosmetic change
  case VolatileId::LOCKED_MOVE: {
    return true;
  }
  // Does nothing
  // case VolatileId::CHARGE:
  // TODO
  // case VolatileId::POWER_SHIFT:
  // case VolatileId::POWER_TRICK:
  // case VolatileId::LASER_FOCUS:
  // case VolatileId::FURY_CUTTER:
  // case VolatileId::STOCKPILE:
  // case VolatileId::HEAL_BLOCK:
  // Skip
  // case VolatileId::ALLY_SWITCH:
  // case VolatileId::HELPING_HAND:
  default:
    break;
  }
  return true;
}
// TODO: finish implementing
bool BattleState::applyOnTryAddVolatile(VolatileId vol, Pokemon &target, Pokemon &source) {
  // Terrain
  if (isGrounded(target, false) && !target.isSemiInvulnerable()) {
    if (terrain == Terrain::ELECTRIC && vol == VolatileId::YAWN) {
      return false;
    } else if (terrain == Terrain::MISTY && vol == VolatileId::CONFUSION) {
      return false;
    }
  }
  // TODO: Safeguard
  // TODO: FocusPunch
  // if(target.focusPunch && vol == VolatileId::FLINCH) {
  //   return false;
  // }
  switch (target.ability) {
  case Ability::INNER_FOCUS: {
    if (vol == VolatileId::FLINCH) {
      return false;
    }
    break;
  }
  case Ability::INSOMNIA:
  case Ability::PURIFYING_SALT:
  case Ability::VITAL_SPIRIT: {
    if (vol == VolatileId::YAWN) {
      return false;
    }
    break;
  }
  case Ability::LEAF_GUARD: {
    if (weather == Weather::DESOLATE_LAND || weather == Weather::SUNNY_DAY) {
      if (vol == VolatileId::YAWN) {
        return false;
      }
    }
    break;
  }
  case Ability::OWN_TEMPO: {
    if (vol == VolatileId::CONFUSION) {
      return false;
    }
    break;
  }
  case Ability::SHIELDS_DOWN: {
    if (target.name == PokeName::MINIOR_METEOR_FORM) {
      if (vol == VolatileId::YAWN) {
        return false;
      }
    }
    break;
  }
  default:
    break;
  }
  return true;
}
// - Set confusion duration
// TODO: finish implementing
bool BattleState::applyOnStartVolatile(VolatileId vol, Pokemon &target, Pokemon &source,
                                       MoveId move) {
  switch (vol) {
  case VolatileId::CONFUSION: {
    target.addConfusion(move == MoveId::AXEKICK);
    break;
  }
  // Does nothing:
  // - FlashFire, SlowStart
  // - Burn, Paralysis, Trapped, MustRecharge
  // - Stall moves, including special versions
  // - AquaRing, Charge, Curse, DefenseCurl, DestinyBond, Imprison, Ingrain, KingsShield,
  // - LaserFocus, LeechSeed, MagnetRise, Minimize, MiracleEye, NoRetreat
  // etc.
  //
  // Do later:
  // - PartiallyTrapped
  // - LockedMove
  // - TwoTurnMove
  // - ChoiceLock
  // - FutureMove
  // - HealReplacement
  // - Attract (onAttract?)
  // - Counter
  // - Bide
  // - Disable
  // - DragonCheer
  // - Electrify
  // - Embargo
  // - Encore
  // - Endure
  // - Fling
  // - FocusEnergy
  // - Foresight
  // - GastroAcid
  // - Grudge
  // - HealBlock
  // - MagicCoat
  // - Nightmare
  // etc.
  // Skip:
  // - AllySwitch, FollowMe, HelpingHand
  default:
    break;
  }
  return true;
}
bool BattleState::applyOnSetStatus(Status status, Pokemon &target, Pokemon &source,
                                   EffectKind effectKind, MoveInstance &moveInst) {
  if (terrain == Terrain::ELECTRIC) {
    if (status == Status::SLEEP && isGrounded(target, false) && !target.isSemiInvulnerable()) {
      return false;
    }
  } else if (terrain == Terrain::MISTY) {
    if (isGrounded(target, false) && !target.isSemiInvulnerable()) {
      return false;
    }
  }
  if (teams[target.side].safeguard) {
    if (effectKind != EffectKind::NO_EFFECT && effectKind != EffectKind::YAWN) {
      // Infiltrator goes through Safeguard
      if (effectKind == EffectKind::MOVE && moveInst.infiltrates && target.side != source.side) {
      } else {
        if (target != source) {
          return false;
        }
      }
    }
  }
  switch (target.ability) {
  case Ability::COMATOSE:
  case Ability::INSOMNIA:
  case Ability::VITAL_SPIRIT: {
    if (status == Status::SLEEP) {
      return false;
    }
    break;
  }
  case Ability::IMMUNITY:
  case Ability::PASTEL_VEIL: {
    if (status == Status::POISON || status == Status::TOXIC) {
      return false;
    }
    break;
  }
  case Ability::LIMBER: {
    if (status == Status::PARALYSIS) {
      return false;
    }
    break;
  }
  case Ability::THERMAL_EXCHANGE:
  case Ability::WATER_BUBBLE:
  case Ability::WATER_VEIL: {
    if (status == Status::BURN) {
      return false;
    }
    break;
  }
  case Ability::LEAF_GUARD: {
    if (weather == Weather::SUNNY_DAY || weather == Weather::DESOLATE_LAND) {
      return false;
    }
    break;
  }
  case Ability::SHIELDS_DOWN: {
    if (target.name == PokeName::MINIOR_METEOR_FORM) {
      return false;
    }
    break;
  }
  default:
    break;
  }
  return true;
}
} // namespace pkmn