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
// -1 disables an option (i.e. randomize or otherwise follow rules)
// roll_val: [0,16) or -1 for randomize
// crit: critIndex
struct DMGCalcOptions {
  int roll_val;
  int crit;
  void fill_crit(const Pokemon &target, const Pokemon &attacker);
  void fill_roll();
};
bool getCrit(const Pokemon &target, const Pokemon &attacker, MoveId, DMGCalcOptions &);
int generateAndApplyDmgRoll(int baseDamage, DMGCalcOptions &);

int checkAndApplySTAB(int baseDamage, const Pokemon &attacker, const Move &);
std::optional<int> getTypeMod(const Pokemon &defender, const Move &);
int applyTypeEffectiveness(int baseDamage, int typeMod);

struct EffectiveStatVals {
  ModifierId attStatName, defStatName;
  int attBoost, defBoost;
};
EffectiveStatVals getEffectiveStats(const Pokemon &attacker, const Pokemon &defender, const Move &);

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