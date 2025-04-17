#include "utils.hpp"

#include <optional>
#include <random>
namespace pkmn {
// If crit stage is not pre-determined, applies onModifyCritRatio.
void DMGCalcOptions::fill_crit(const Pokemon &target, const Pokemon &attacker) {
  if (crit < 0) {
    crit = 1;
    // onModifyCritRatio():
    // Note that crit stage of 5 is legacy: it'll end up being capped at 4
    // Merciless, Super Luck
    if (attacker.ability == Ability::MERCILESS) {
      if (target.status == Status::POISON || target.status == Status::TOXIC) {
        crit = 5;
      }
    }
    if (attacker.ability == Ability::SUPER_LUCK) {
      crit++;
    }
    // Leek, Stick, Lucky Punch, Razor Claw, Scope Lens
    if (attacker.species == Species::FARFETCHD || attacker.species == Species::SIRFETCHD) {
      if (attacker.has_item(Item::LEEK)) {
        crit += 2;
      }
    }
    if (attacker.species == Species::FARFETCHD && attacker.has_item(Item::STICK)) {
      crit += 2;
    }
    if (attacker.species == Species::CHANSEY && attacker.has_item(Item::LUCKY_PUNCH)) {
      crit += 2;
    }
    if (attacker.has_item(Item::RAZOR_CLAW) || attacker.has_item(Item::SCOPE_LENS)) {
      crit++;
    }
    // Dragon Cheer, Focus Energy, Laser Focus
    // if(attacker.hasDragonCheer) {
    //   crit ++;
    //   if(attacker.has_type(Type::DRAGON)) {
    //     crit++;
    //   }
    // }
    // if(attacker.hasFocusEnergy) {
    //   crit += 2;
    // }
    // if(attacker.hasLaserFocus) {
    //   crit = 5;
    // }
  }
}
// If roll value (in [0, 16)) is not pre-determined, executes the damage roll.
void DMGCalcOptions::fill_roll() {
  if (roll_val < 0) {
    roll_val = math::random(16);
  }
}
// Given the crit index (options.crit):
// Check if the move hit, if so use RNG to calc if the move crits or not.
bool getCrit(const Pokemon &target, const Pokemon &attacker, MoveId move, DMGCalcOptions &options) {
  // Run and Bun has 1/16 base crit chance
  static const std::array<int, 5> critMult = {0, 16, 8, 2, 1}; // Crit chance at each stage = 1/n
  options.fill_crit(target, attacker);                   // onModifyCrit: leaves options.crit > 0
  options.crit = std::max(0, std::min(4, options.crit)); // Clamp to valid stage
  // Q: getMoveHitData...doesn't do anything?
  // Calculate if the move crits or not
  static const std::vector<MoveId> noCrit = {MoveId::COUNTER, MoveId::FLAIL, MoveId::REVERSAL,
                                             MoveId::DOOMDESIRE,
                                             MoveId::FUTURESIGHT}; // Some moves never crit.
  static const std::vector<MoveId> willCrit = {MoveId::FROSTBREATH, MoveId::STORMTHROW,
                                               MoveId::SURGINGSTRIKES,
                                               MoveId::WICKEDBLOW}; // Others always crit.
  std::optional<bool> moveWillCrit = std::nullopt;
  if (std::find(noCrit.begin(), noCrit.end(), move) != noCrit.end()) {
    moveWillCrit = std::optional<bool>{false};
  }
  if (std::find(willCrit.begin(), willCrit.end(), move) != willCrit.end()) {
    moveWillCrit = std::optional<bool>{true};
  }
  if (moveWillCrit == std::nullopt) {
    if (options.crit) { // True unless manually set to no-crit
      return math::randomChance(1, critMult[options.crit]);
    } else {
      std::cerr << "[getCrit] options.crit manually set to zero." << std::endl;
      return false;
    }
  }
  return *moveWillCrit;
}
int generateAndApplyDmgRoll(int baseDamage, DMGCalcOptions &options) {
  options.fill_roll();
  return baseDamage * (options.roll_val + 85) / 100;
}
int checkAndApplySTAB(int baseDamage, const Pokemon &attacker, const Move &move) {
  // There's weird stuff with '???' type from e.g. Roost dropping the sole Flying type
  int stab_numer = 1, stab_denom = 1;
  // isSTAB = move.forceSTAB || pokemon.hasType(type) || pokemon.getTypes(false,
  // true).includes(type); // Here forceSTAB is from Pledge combos
  if (attacker.has_type(move.type)) {
    // STAB multiplier is 1.5
    stab_numer = 3, stab_denom = 2;
  }
  // TODO: onModifySTAB
  return applyModifier(baseDamage, stab_numer, stab_denom);
}
// If move is used to attack the Pokemon defender, is it 0.5x? 2x? 1x? 4x? immune?
std::optional<int> getTypeMod(const Pokemon &defender, const Move &move) {
  // Note: runEffectiveness is b/c Arceus/Silvally and Tera
  std::optional<int> typeMod{0};
  for (Type t : defender.types) {
    if (t != Type::NO_TYPE) {
      const int newTypeMod = getTypeEffectiveness(move.type, t);
      if (newTypeMod < -1) { // Immune
        typeMod = std::nullopt;
      } else if (typeMod != std::nullopt) {
        *typeMod += newTypeMod;
      }
    }
  }
  return typeMod;
}
// Apply a type effectiveness modifier from getTypeMod() to a base damage amount.
int applyTypeEffectiveness(int baseDamage, int typeMod) {
  if (typeMod > 0) {
    // std::cout << "Super effective!\n";
    for (; typeMod > 0; (typeMod)--) {
      baseDamage *= 2;
    }
  } else if (typeMod < 0) {
    // std::cout << "Not very effective!\n";
    for (; typeMod < 0; (typeMod)++) {
      baseDamage /= 2;
    }
  }
  return baseDamage;
}
// Return the categories to use for attack/defense in damage calculation.
EffectiveStatVals getEffectiveStats(const Pokemon &attacker, const Pokemon &defender,
                                    const Move &move) {
  EffectiveStatVals ret;
  // Physical/Special moves should all have Power! (I checked this)
  if (move.category == MoveCategory::PHYSICAL) {
    ret.attStatName = ModifierId::ATTACK;
    ret.defStatName = ModifierId::DEFENSE;
    ret.attBoost = attacker.boosts.att;
    ret.defBoost = defender.boosts.def;
  } else if (move.category == MoveCategory::SPECIAL) {
    ret.attStatName = ModifierId::SPATT;
    ret.defStatName = ModifierId::SPDEF;
    ret.attBoost = attacker.boosts.spatt;
    ret.defBoost = defender.boosts.spdef;
  } else {
    // ERROR
    std::cerr << "[getEffectiveStats] Trying to call getEffectiveStats with invalid category."
              << std::endl;
  }
  // override__Stat
  switch (move.id) {
  case MoveId::BODYPRESS: {
    ret.attStatName = ModifierId::DEFENSE;
    ret.attBoost = attacker.boosts.def;
    break;
  }
  case MoveId::PSYSHOCK:
  case MoveId::PSYSTRIKE:
  case MoveId::SECRETSWORD: {
    ret.defStatName = ModifierId::DEFENSE;
    ret.defBoost = attacker.boosts.def;
    break;
  default: {
    break;
  }
  }
  }
  return ret;
}
// Assumes inputs are non-negative ints (and denominator is positive)
// Note: uses integer division
// Technically should be uint32_t operations but I think it's fine here
// Common numbers:
// 1352/4096 ~ 0.33
// 3277/4096 ~ 0.8
// 4505/4096 ~ 1.1
// 4915/4096 ~ 1.2
// 5325/4096 ~ 1.3
int applyModifier(const int val, const int numerator, const int denominator) {
  const int modifier = 4096 * numerator / denominator;
  return (val * modifier + 2048 - 1) / 4096;
}
int boostStatVal(int statVal, int boostIndex) {
  boostIndex = std::max(-6, std::min(6, boostIndex));
  if (boostIndex >= 0) {
    statVal = statVal * (boostIndex + 2) / 2;
  } else {
    statVal = statVal * 2 / (boostIndex + 2);
  }
  return statVal;
}
bool isBerry(Item item) {
  static const std::vector<Item> berries = {
      Item::AGUAV_BERRY,  Item::APICOT_BERRY, Item::ASPEAR_BERRY,  Item::BABIRI_BERRY,
      Item::BELUE_BERRY,  Item::BLUK_BERRY,   Item::CHARTI_BERRY,  Item::CHERI_BERRY,
      Item::CHESTO_BERRY, Item::CHILAN_BERRY, Item::CHOPLE_BERRY,  Item::COBA_BERRY,
      Item::COLBUR_BERRY, Item::CORNN_BERRY,  Item::CUSTAP_BERRY,  Item::DURIN_BERRY,
      Item::ENIGMA_BERRY, Item::FIGY_BERRY,   Item::GANLON_BERRY,  Item::GREPA_BERRY,
      Item::HABAN_BERRY,  Item::HONDEW_BERRY, Item::IAPAPA_BERRY,  Item::JABOCA_BERRY,
      Item::KASIB_BERRY,  Item::KEBIA_BERRY,  Item::KEE_BERRY,     Item::KELPSY_BERRY,
      Item::LANSAT_BERRY, Item::LEPPA_BERRY,  Item::LIECHI_BERRY,  Item::LUM_BERRY,
      Item::MAGO_BERRY,   Item::MAGOST_BERRY, Item::MARANGA_BERRY, Item::MICLE_BERRY,
      Item::NANAB_BERRY,  Item::NOMEL_BERRY,  Item::OCCA_BERRY,    Item::ORAN_BERRY,
      Item::PAMTRE_BERRY, Item::PASSHO_BERRY, Item::PAYAPA_BERRY,  Item::PECHA_BERRY,
      Item::PERSIM_BERRY, Item::PETAYA_BERRY, Item::PINAP_BERRY,   Item::POMEG_BERRY,
      Item::QUALOT_BERRY, Item::RABUTA_BERRY, Item::RAWST_BERRY,   Item::RAZZ_BERRY,
      Item::RINDO_BERRY,  Item::ROSELI_BERRY, Item::ROWAP_BERRY,   Item::SALAC_BERRY,
      Item::SHUCA_BERRY,  Item::SITRUS_BERRY, Item::SPELON_BERRY,  Item::STARF_BERRY,
      Item::TAMATO_BERRY, Item::TANGA_BERRY,  Item::WACAN_BERRY,   Item::WATMEL_BERRY,
      Item::WEPEAR_BERRY, Item::WIKI_BERRY,   Item::YACHE_BERRY,
  };
  return std::find(berries.begin(), berries.end(), item) != berries.end();
}
bool isGem(Item item) {
  static const std::vector<Item> gems = {
      Item::BUG_GEM,     Item::ICE_GEM,      Item::DARK_GEM,     Item::FIRE_GEM,   Item::ROCK_GEM,
      Item::GHOST_GEM,   Item::GRASS_GEM,    Item::FAIRY_GEM,    Item::STEEL_GEM,  Item::WATER_GEM,
      Item::GROUND_GEM,  Item::DRAGON_GEM,   Item::FLYING_GEM,   Item::NORMAL_GEM, Item::POISON_GEM,
      Item::PSYCHIC_GEM, Item::ELECTRIC_GEM, Item::FIGHTING_GEM,
  };
  return std::find(gems.begin(), gems.end(), item) != gems.end();
}
// Rule of thumb: forme-changing and/or type-changing and/or ability-changing can't be traced.
// Also OP: AsOne and random: Comatose.
bool noTrace(Ability ability) {
  static const std::vector<Ability> unTraceable = {
      Ability::AS_ONE_GLASTRIER, Ability::AS_ONE_GLASTRIER,
      Ability::BATTLE_BOND,      Ability::COMATOSE,
      Ability::COMMANDER,        Ability::DISGUISE,
      Ability::FLOWER_GIFT,      Ability::FORECAST,
      Ability::HUNGER_SWITCH,    Ability::ICE_FACE,
      Ability::ILLUSION,         Ability::IMPOSTER,
      Ability::MULTITYPE,        Ability::NEUTRALIZING_GAS,
      Ability::POWER_CONSTRUCT,  Ability::POWER_OF_ALCHEMY,
      Ability::RECEIVER,         Ability::RKS_SYSTEM,
      Ability::SCHOOLING,        Ability::SHIELDS_DOWN,
      Ability::STANCE_CHANGE,    Ability::TRACE,
      Ability::ZEN_MODE,         Ability::GULP_MISSILE,
  };
  return std::find(unTraceable.begin(), unTraceable.end(), ability) != unTraceable.end();
}
Stats get_nature_contrib(const Nature nature) {
  Stats ret{};
  const std::vector<Nature> att_boost = {Nature::LONELY, Nature::ADAMANT, Nature::NAUGHTY,
                                         Nature::BRAVE};
  const std::vector<Nature> def_boost = {Nature::BOLD, Nature::IMPISH, Nature::LAX,
                                         Nature::RELAXED};
  const std::vector<Nature> spatt_boost = {Nature::MODEST, Nature::MILD, Nature::RASH,
                                           Nature::QUIET};
  const std::vector<Nature> spdef_boost = {Nature::CALM, Nature::GENTLE, Nature::CAREFUL,
                                           Nature::SASSY};
  const std::vector<Nature> spd_boost = {Nature::TIMID, Nature::HASTY, Nature::JOLLY,
                                         Nature::NAIVE};
  const std::vector<Nature> att_drop = {Nature::BOLD, Nature::MODEST, Nature::CALM, Nature::TIMID};
  const std::vector<Nature> def_drop = {Nature::LONELY, Nature::MILD, Nature::GENTLE,
                                        Nature::HASTY};
  const std::vector<Nature> spatt_drop = {Nature::ADAMANT, Nature::IMPISH, Nature::CAREFUL,
                                          Nature::JOLLY};
  const std::vector<Nature> spdef_drop = {Nature::NAUGHTY, Nature::LAX, Nature::RASH,
                                          Nature::NAIVE};
  const std::vector<Nature> spd_drop = {Nature::BRAVE, Nature::RELAXED, Nature::QUIET,
                                        Nature::SASSY};
  if (std::find(att_boost.begin(), att_boost.end(), nature) != att_boost.end()) {
    ret.att = 1;
  } else if (std::find(def_boost.begin(), def_boost.end(), nature) != def_boost.end()) {
    ret.def = 1;
  } else if (std::find(spatt_boost.begin(), spatt_boost.end(), nature) != spatt_boost.end()) {
    ret.spatt = 1;
  } else if (std::find(spdef_boost.begin(), spdef_boost.end(), nature) != spdef_boost.end()) {
    ret.spdef = 1;
  } else if (std::find(spd_boost.begin(), spd_boost.end(), nature) != spd_boost.end()) {
    ret.spd = 1;
  }
  if (std::find(att_drop.begin(), att_drop.end(), nature) != att_drop.end()) {
    ret.att = -1;
  } else if (std::find(def_drop.begin(), def_drop.end(), nature) != def_drop.end()) {
    ret.def = -1;
  } else if (std::find(spatt_drop.begin(), spatt_drop.end(), nature) != spatt_drop.end()) {
    ret.spatt = -1;
  } else if (std::find(spdef_drop.begin(), spdef_drop.end(), nature) != spdef_drop.end()) {
    ret.spdef = -1;
  } else if (std::find(spd_drop.begin(), spd_drop.end(), nature) != spd_drop.end()) {
    ret.spd = -1;
  }
  return ret;
}
} // namespace pkmn

namespace pkmn::math {
// int odd_round(int a, int b, int c) {
//   // Return (a*b)/c, rounding to the nearest integer but rounding down at 0.5
//   double result = static_cast<double>(a * b) / c;
//   // Round to the nearest integer, but round down at exactly 0.5
//   int rounded = (result - std::floor(result) == 0.5) ? std::floor(result) : std::round(result);
//   return static_cast<int>(rounded);
// }
// Return a random 32-bit integer.
uint32_t randomUint32() {
  // Here I just use ChatGPT-provided RNG code template
  // TODO: Replace with faithful PRNG
  static std::random_device rd;  // Seed source (may be slow)
  static std::mt19937 gen(rd()); // Mersenne Twister PRNG
  static std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);
  return dist(gen);
}
} // namespace pkmn::math