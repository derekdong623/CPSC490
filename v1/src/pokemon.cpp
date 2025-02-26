#include "pokemon.hpp"
#include "pokemon_enums.hpp"
namespace pkmn {
int get_stat(int base, int iv, int ev, int lvl, int nature_change) {
  // Helper function getting non-HP stats
  double nature_factor = (10. + nature_change) / 10.;
  return (int)((double)(((2 * base + iv + (ev / 4)) * lvl / 100) + 5) * nature_factor);
}
Stats Pokemon::get_stats(const Stats &base, const Stats &nature, const Stats &IVs, const Stats &EVs,
                         const int lvl, const bool is_shedinja) {
  // For initializing Pokemon stats
  // base, IVs, and EVs should be non-negative. lvl should be in [1,100]. IVs and EVs should be
  // legal.
  // TODO: implement reading base Stats from database.
  // nature should be all-zero or a single 1 and a single -1.
  Stats ret{};
  ret.hp = ((2 * base.hp + IVs.hp + (EVs.hp / 4)) * lvl / 100) + lvl + 10;
  // Shedinja always has 1 hp.
  if (is_shedinja) {
    ret.hp = 1;
  }
  ret.att = get_stat(base.att, IVs.att, EVs.att, lvl, nature.att);
  ret.def = get_stat(base.def, IVs.def, EVs.def, lvl, nature.def);
  ret.spatt = get_stat(base.spatt, IVs.spatt, EVs.spatt, lvl, nature.spatt);
  ret.spdef = get_stat(base.spdef, IVs.spdef, EVs.spdef, lvl, nature.spdef);
  ret.spd = get_stat(base.spd, IVs.spd, EVs.spd, lvl, nature.spd);
  return ret;
}
Stats get_base_stats(const Species species) {
  // TODO
  Stats ret{};
  if (species == Species::ABOMASNOW) {
    ret.hp = 1;
  }
  return ret;
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