#pragma once

#include "pokemon.hpp"
#include <optional>
namespace pkmn {
struct Team {
    std::optional<Pokemon> pkmn[6]; // Active member in slot 0
    // Tailwind, Reflect, etc. turns
};
struct ActionState {
    bool moved;
    bool to_switch;
    ActionState() : moved(false), to_switch(false) {}
};
struct MoveOption {
    bool move[4] = {};
    bool swap[5] = {};
};
struct BattleState {
    Team teams[2]; // Player in slot 0
    MoveOption move_options[2];
    ActionState action_states[2];
    // Weather/terrain, etc.
    void set_move_options();
};

// Assumes inputs are non-negative ints (and denominator is positive)
int applyModifier(const int val, const int numerator, const int denominator);

// get_effective_*_stat should ignore stat changes (modifiers applied afterward)
int get_effective_attack_stat(const Pokemon& attacker, const Move &move);
int get_effective_defense_stat(const Pokemon& defender, const Move &move);
// Used primarily as a sanity-checker -- don't think too hard about it 
// (e.g. we don't worry about multi-hits)
// roll_val in [0,16)
int get_deterministic_dmg_roll(const Pokemon& attacker, const Pokemon& defender, const Move &move, bool crit, int roll_val);

// BattleState runMove(BattleState &start_state, int turn, MoveOption &choice);
} // namespace pkmn