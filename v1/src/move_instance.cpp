#include "move_instance.hpp"
#include "utils.hpp"
namespace pkmn {
bool MoveInstance::onModifyType(Pokemon const &pokemon) {
  static const std::vector<MoveId> noModifyType = {
      MoveId::JUDGMENT,    MoveId::MULTIATTACK,  MoveId::NATURALGIFT, MoveId::REVELATIONDANCE,
      MoveId::TECHNOBLAST, MoveId::TERRAINPULSE, MoveId::WEATHERBALL};
  if (std::find(noModifyType.begin(), noModifyType.end(), id) == noModifyType.end()) {
    if (pokemon.ability == Ability::NORMALIZE) {
      moveData.type = Type::NORMAL;
    }
    if (moveData.type == Type::NORMAL) {
      switch (pokemon.ability) {
      case Ability::AERILATE:
        moveData.type = Type::FLYING;
      case Ability::GALVANIZE:
        moveData.type = Type::ELECTRIC;
      case Ability::PIXILATE:
        moveData.type = Type::FAIRY;
      case Ability::REFRIGERATE:
        moveData.type = Type::ICE;
      default:
        break;
      }
    }
    // typeChangerBoosted = this.effect;
  }
  if (pokemon.ability == Ability::LIQUID_VOICE && moveData.sound)
    moveData.type = Type::WATER;
  // TODO: Judgement, MultiAttack, TechnoBlast
  // TerrainPulse
  // WeatherBall
  // RagingBull
  // RevelationDance
  // NaturalGift
  // TODO: AuraWheel, HiddenPower
  // TODO: Electrify, IonDeluge
  return true;
}
bool MoveInstance::onModifyMove(Pokemon const &pokemon) {
  // PRIORITY 1
  // PropellerTail and Stalwart: do at getTarget level
  // StanceChange: formeChange
  // PRIORITY 0
  // TODO: GorillaTactics and Choice items...why GorillaTactics implemented a bit different?
  // Illuminate: N/A, only Gen 9
  // move.infiltrates = true; // Infiltrator
  // move.ignoreEvasion = true; // KeenEye
  // move.contact = false; // LongReach, PunchingGlove+punch
  // move.ignoreAbility = true; // MoldBreaker, MyceliumMight+Status, Teravolt, Turboblaze,
  // move.protect = false; // UnseenFist
  // if(move.moveData.defrost) { // Defrosting
  //   pokemon.clearStatus;
  // }
  // SkillLink: max roll or, for multiaccuracy, one accuracy check
  // LoadedDice: delete multiaccuracy only?
  // BeatUp: very complicated
  // BleakWindstorm, SandsearStorm, WildboltStorm: rain/primordial -> always hits
  // Blizzard: hail -> always hits
  // Growth: sun/desolate -> double boosts
  // Hurricane, Thunder: rain/primordial -> always, sun/desolate -> 50 acc
  // Curse: check Gen8 mod
  // ExpandingForce: N/A in singles
  // Pledge: N/A in singles
  // Gravity: Might cancel move by returning false
  // HealBlock: complicated
  // IceBall/Rollout: complicated
  // LightThatBurnsTheSky: N/A in Run&Bun
  // Magnitude: roll for power
  // PhotonGeyser: swap to physical if higher att
  // Present: skip for now
  // Pursuit: always hits if target being called back or switch flag
  // SecretPower: complicated
  // ShellsideArm: complicated
  // SkyDrop: complicated
  // Struggle: give NoneType
  // TeraBlast, TeraStarstorm: N/A, only Gen 9
  // if(field.terrain && pokemon.isGrounded()) {basePower *= 2;} // TerrainPulse
  // if(field.weather != Weather::NONE) {basePower *= 2;} // WeatherBall
  // ThroatChop
  // WonderRoom: extremely complicated
  // PRIORITY -1
  // Stench, KingsRock, RazorFang: push flinch secondary
  // PRIORITY -2
  // SereneGrace: double chance of secondary
  // SheerForce: delete secondaries and self-damage etc.
  // PRIORITY -5
  // Mind's Eye: N/A, only Gen 9 Bloodmoon Ursaluna
  // Scrappy
  return true;
}
//
bool MoveInstance::breaksProtect() {
  static const std::vector<MoveId> listOfMoves = {
      MoveId::FEINT,        MoveId::HYPERSPACEFURY, MoveId::HYPERSPACEHOLE,
      MoveId::PHANTOMFORCE, MoveId::SHADOWFORCE,
  };
  return std::find(listOfMoves.begin(), listOfMoves.end(), id) != listOfMoves.end();
}
// Could be 2-5 or strictly 2/3-hitting
bool MoveInstance::isMultiHit() {
  static const std::vector<MoveId> multihitMoves = {
      MoveId::ARMTHRUST,   MoveId::BARRAGE,        MoveId::BONERUSH,       MoveId::BULLETSEED,
      MoveId::COMETPUNCH,  MoveId::DOUBLESLAP,     MoveId::FURYATTACK,     MoveId::FURYSWIPES,
      MoveId::ICICLESPEAR, MoveId::PINMISSILE,     MoveId::ROCKBLAST,      MoveId::SCALESHOT,
      MoveId::SPIKECANNON, MoveId::TAILSLAP,       MoveId::WATERSHURIKEN,  MoveId::BONEMERANG,
      MoveId::DOUBLEHIT,   MoveId::DOUBLEIRONBASH, MoveId::DOUBLEKICK,     MoveId::DRAGONDARTS,
      MoveId::DUALCHOP,    MoveId::DUALWINGBEAT,   MoveId::GEARGRIND,      MoveId::TACHYONCUTTER,
      MoveId::TWINBEAM,    MoveId::TWINEEDLE,      MoveId::SURGINGSTRIKES, MoveId::TRIPLEAXEL,
      MoveId::TRIPLEDIVE,  MoveId::TRIPLEKICK,
  };
  return std::find(multihitMoves.begin(), multihitMoves.end(), id) != multihitMoves.end();
}
// Explicit exceptions
bool MoveInstance::isNoParentalBond() {
  static const std::vector<MoveId> no_pb_moves = {
      MoveId::DRAGONDARTS, MoveId::ENDEAVOR, MoveId::EXPLOSION, MoveId::FINALGAMBIT,
      MoveId::FLING,       MoveId::ICEBALL,  MoveId::ROLLOUT,   MoveId::SELFDESTRUCT,
  };
  return std::find(no_pb_moves.begin(), no_pb_moves.end(), id) != no_pb_moves.end();
}
// Two-turn or SkyDrop
bool MoveInstance::isCharge() {
  static const std::vector<MoveId> chargeMoves = {
      MoveId::BOUNCE,     MoveId::DIG,          MoveId::DIVE,      MoveId::ELECTROSHOT,
      MoveId::FLY,        MoveId::FREEZESHOCK,  MoveId::GEOMANCY,  MoveId::ICEBURN,
      MoveId::METEORBEAM, MoveId::PHANTOMFORCE, MoveId::RAZORWIND, MoveId::SHADOWFORCE,
      MoveId::SKULLBASH,  MoveId::SKYATTACK,    MoveId::SKYDROP,   MoveId::SOLARBEAM,
      MoveId::SOLARBLADE,
  };
  return std::find(chargeMoves.begin(), chargeMoves.end(), id) != chargeMoves.end();
}
// Regular recoil moves
std::pair<int, int> MoveInstance::getRecoil() {
  static std::unordered_map<MoveId, std::pair<int, int>> recoilMoves = {
      {MoveId::BRAVEBIRD, {33, 100}},  {MoveId::DOUBLEEDGE, {33, 100}},
      {MoveId::FLAREBLITZ, {33, 100}}, {MoveId::VOLTTACKLE, {33, 100}},
      {MoveId::WAVECRASH, {33, 100}},  {MoveId::WOODHAMMER, {33, 100}},
      {MoveId::HEADSMASH, {1, 2}},     {MoveId::LIGHTOFRUIN, {1, 2}},
      {MoveId::HEADCHARGE, {1, 4}},    {MoveId::SUBMISSION, {1, 4}},
      {MoveId::TAKEDOWN, {1, 4}},      {MoveId::WILDCHARGE, {1, 4}},
  };
  auto it = recoilMoves.find(id);
  return it != recoilMoves.end() ? it->second : std::make_pair(0, 0);
}
// DoomDesire and FutureSight
bool MoveInstance::isFuture() { return id == MoveId::DOOMDESIRE || id == MoveId::FUTURESIGHT; }
// Calls another move, e.g. Assist
bool MoveInstance::isCalling() {
  static const std::vector<MoveId> callMoves = {
      MoveId::ASSIST,     MoveId::COPYCAT,     MoveId::MEFIRST,   MoveId::METRONOME,
      MoveId::MIRRORMOVE, MoveId::NATUREPOWER, MoveId::SLEEPTALK,
  };
  return std::find(callMoves.begin(), callMoves.end(), id) != callMoves.end();
}
// Self-explanatory
bool MoveInstance::isOHKO() {
  // Note: should have special case for Ice-type!
  static const std::vector<MoveId> ohkos = {MoveId::FISSURE, MoveId::GUILLOTINE, MoveId::HORNDRILL,
                                      MoveId::SHEERCOLD};
  return std::find(ohkos.begin(), ohkos.end(), id) != ohkos.end();
}
// Self-explanatory
bool MoveInstance::isSleepUsable() {
  return id == MoveId::SLEEPTALK || id == MoveId::SNORE;
}
// Compute numHits, with Skill Link and Loaded Dice potential logic.
// Without PopulationBomb or BeatUp, every 2-5 has the same randomization.
// Accuracy check type defined by move.multiaccuracy.
int MoveInstance::getNumHits(Pokemon const &pokemon) {
  // Moves that hit 2-5 times.
  static const std::vector<MoveId> hit_2_5{
      MoveId::ARMTHRUST,   MoveId::BARRAGE,    MoveId::BONERUSH,     MoveId::BULLETSEED,
      MoveId::COMETPUNCH,  MoveId::DOUBLESLAP, MoveId::FURYATTACK,   MoveId::FURYSWIPES,
      MoveId::ICICLESPEAR, MoveId::PINMISSILE, MoveId::ROCKBLAST,    MoveId::SCALESHOT,
      MoveId::SPIKECANNON, MoveId::TAILSLAP,   MoveId::WATERSHURIKEN};
  // Moves that hit twice.
  static const std::vector<MoveId> hit_2{
      MoveId::BONEMERANG,    MoveId::DOUBLEHIT, MoveId::DOUBLEIRONBASH, MoveId::DOUBLEKICK,
      MoveId::DRAGONDARTS,   MoveId::DUALCHOP,  MoveId::DUALWINGBEAT,   MoveId::GEARGRIND,
      MoveId::TACHYONCUTTER, MoveId::TWINBEAM,  MoveId::TWINEEDLE};
  // Moves that hit three times.
  static const std::vector<MoveId> hit_3{MoveId::SURGINGSTRIKES, MoveId::TRIPLEAXEL, MoveId::TRIPLEDIVE,
                                   MoveId::TRIPLEKICK};
  // Population bomb is a special case.
  if (std::find(hit_2.begin(), hit_2.end(), id) != hit_2.end()) {
    return 2;
  }
  if (std::find(hit_3.begin(), hit_3.end(), id) != hit_3.end()) {
    return 3;
  }
  if (std::find(hit_2_5.begin(), hit_2_5.end(), id) != hit_2_5.end()) {
    // 35-35-15-15 out of 100 for 2-3-4-5 hits
    double r = math::random();
    int numHits = 2;
    if (r > 0.35) {
      numHits++;
    }
    if (r > 0.7) {
      numHits++;
    }
    if (r > 0.85) {
      numHits++;
    }
    if (numHits < 4 && pokemon.has_item(Item::LOADED_DICE)) {
      return 5 - math::random(2);
    }
  }
  return 1;
}
// "multiaccuracy" moves: Population Bomb, Triple Axel, and Triple Kick.
// These moves run accuracy checks on *every* hit.
// The Loaded Dice item and Skill Link ability turns it off (one acc check).
bool MoveInstance::multiaccCheck(Pokemon const &target, Pokemon const &pokemon) {
  static const std::vector<MoveId> multiacc_moves{MoveId::POPULATIONBOMB, MoveId::TRIPLEAXEL,
                                            MoveId::TRIPLEKICK};
  bool multiaccuracy =
      std::find(multiacc_moves.begin(), multiacc_moves.end(), id) != multiacc_moves.end();
  if (pokemon.has_item(Item::LOADED_DICE) || pokemon.has_ability(Ability::SKILL_LINK)) {
    multiaccuracy = false;
  }
  return multiaccuracy;
}
// Checks relevant Accuracy and Evasion boosts and onAccuracy (e.g.
// conditions like Sandstorm+SandVeil or Gravity).
// **Assumes the move has numerical accuracy -- do acc check bypasses first!
bool MoveInstance::getBasicAcc(Pokemon const &target, Pokemon const &pokemon, bool multiacc) {
  int acc = *moveData.accuracy; // Should not error b/c bypass check is first!
  // Accuracy boosts
  int accBoost = std::min(6, std::max(-6, pokemon.boosts.acc));
  int evaBoost = 0;
  if (!(target.foresight || target.miracleeye) || target.boosts.eva <= 0) {
    evaBoost = std::min(6, std::max(-6, target.boosts.eva));
  }
  if (multiacc) {
    if (accBoost > 0) {
      acc *= (3 + accBoost) / 3.;
    } else if (accBoost < 0) {
      acc /= (3 - accBoost) / 3.;
    }
    if (evaBoost > 0) {
      acc /= (3 + evaBoost) / 3.;
    } else if (evaBoost < 0) {
      acc *= (3 - evaBoost) / 3.;
    }
  } else {
    int boost = accBoost - evaBoost;
    if (boost > 0) {
      acc *= (3 + boost) / 3.;
    } else if (boost < 0) {
      acc *= 3. / (3 - boost);
    }
  }
}
} // namespace pkmn