#pragma once

#include "ref.hpp"
#include <algorithm>
#include <string>
#include <vector>
namespace pkmn {
Stats get_nature_contrib(const Nature nature);
struct Pokemon {
  Species name;
  int lvl;
  Stats init_stats, stats; // Some moves e.g. might swap attack/spatt or smth
  int current_hp;
  Type types[2];
  // Boosts, statuses, move-changes/transform data
  std::optional<Move> moves[4];
  // COMPUTING STATS
  Stats get_stats(const Stats &base, const Stats &nature, const Stats &IVs, const Stats &EVs,
                  const int lvl, const bool is_shedinja);
  Pokemon() : name(Species::NONE), lvl(0), init_stats{}, moves{} {}
  Pokemon(Species name_, int lvl_, const Nature nature, const Stats &IVs, const Stats &EVs);
  void set_move_set(std::optional<MoveId> ids[4]); // MOVE SET HELPER
  bool has_type(Type t) const;
};
} // namespace pkmn