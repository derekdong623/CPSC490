#pragma once

#include "ref.hpp"
#include <algorithm>
#include <string>
#include <vector>
namespace pkmn {
Stats get_nature_contrib(const Nature nature);
struct Pokemon {
  const Species name;
  const int lvl;
  Stats init_stats{};
  Stats get_stats(const Stats &base, const Stats &nature, const Stats &IVs, const Stats &EVs,
                  const int lvl, const bool is_shedinja);
  Pokemon(Species name_, int lvl_, const Nature nature, const Stats &IVs, const Stats &EVs)
      : name(name_), lvl(lvl_) {
    init_stats = get_stats(base_stat_dict.dict[name_], get_nature_contrib(nature), IVs, EVs, lvl_,
                           name_ == Species::SHEDINJA);
  }
};
} // namespace pkmn