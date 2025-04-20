#pragma once

#include "pokemon.hpp"
#include "pokemon_enums.hpp"
#include <cmath>
namespace pkmn::math {
uint32_t randomUint32();
// Return a random double in [0,1).
inline double random() { return static_cast<double>(randomUint32()) / std::pow(2.0, 32); }
// Return a random int in [0,x). Requires x > 0.
inline int random(int x) { return static_cast<int>(static_cast<double>(x) * random()); }
// Return a random int in [x, y). Requires y > x.
inline int random(int x, int y) { return random(y - x) + x; }
// TODO: replace with faithful PRNG
inline bool randomChance(int numerator, int denominator) {
  // return false; // Pokemon Showdown has an option to force random chance
  return random(denominator) < numerator;
}
} // namespace pkmn::math

namespace pkmn {
// NONE, FALSE, or TRUE
enum class TriState {
  NONE,
  FALSE,
  TRUE,
};
// -1 disables an option (i.e. randomize or otherwise follow rules)
// roll_val: [0,16) or -1 for randomize
// crit: critIndex
struct DMGCalcOptions {
  int roll_val = -1;
  int crit = -1;
  void fill_crit(const Pokemon &target, const Pokemon &attacker);
  void fill_roll();
};
bool getCrit(const Pokemon &target, const Pokemon &attacker, MoveId, DMGCalcOptions &);
int generateAndApplyDmgRoll(int baseDamage, DMGCalcOptions &);

int checkAndApplySTAB(int baseDamage, const Pokemon &attacker, const Move &);
std::optional<int> getTypeMod(const Pokemon &defender, const Type &);
int applyTypeEffectiveness(int baseDamage, int typeMod);

struct EffectiveStatVals {
  ModifierId attStatName, defStatName;
  int attBoost, defBoost;
};
EffectiveStatVals getEffectiveStats(const Pokemon &attacker, const Pokemon &defender, const Move &);

// To check numeric: first check (initialized && !fail && !succ)
// - numeric must be non-negative
// To check success: first check (initialized && !fail)
// To check fail: first check (initialized)
// To check undefined (i.e. uninitialized): directly check
struct DamageResultState {
  int damageDealt = -1;     // PokemonShowdown numeric
  bool fail = false;        // PokemonShowdown false/null
  bool succ = false;        // PokemonShowdown true
  bool initialized = false; // PokemonShowdown undefined

  DamageResultState() {}
  DamageResultState(int x) { initialized = true, fail = false, succ = false, damageDealt = x; }
  void set_numeric(int x) { initialized = true, fail = false, succ = false, damageDealt = x; }
  void set_succ() {
    if (damageDealt < 0)
      initialized = true, fail = false, succ = true;
  }
  void set_fail() {
    if (damageDealt < 0 && !succ)
      initialized = true, fail = true;
  }
};

struct SecondaryEffect {
  Secondary kind = Secondary::NONE;
  std::optional<int> chance = std::nullopt; // nullopt == N/A chance, i.e. always applies
  std::map<ModifierId, int> boosts;
  std::map<ModifierId, int> selfBoosts;
  Status status = Status::NO_STATUS;
  VolatileId vol = VolatileId::NONE;
};

Stats get_nature_contrib(const Nature nature);
// stat: raw "base" stat value
// boostIndex: in [-6, 6]
int boostStatVal(int statVal, int boostIndex);
int applyModifier(const int val, const int numerator, const int denominator);
// Return whether or not the item is a berry
bool isBerry(Item item);
// Return whether or not the item is a gem
bool isGem(Item item);
bool noTrace(Ability);
} // namespace pkmn