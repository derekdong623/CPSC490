#include "battle_state.hpp"

#include <optional>
namespace pkmn {
void BattleState::set_move_options() {
    for(int i=0; i<2; i++) {
        move_options[i] = {};
        for(int j=0; j<4; j++) {
            if(teams[i].pkmn[0]->moves[j] != std::nullopt) {
                // additional logic for e.g. Struggle, Disable, Fly, Outrage, etc.
                move_options[i].move[j] = true;
            }
        }
        for(int j=1; j<6; j++) {
            if(teams[i].pkmn[j] != std::nullopt) {
                // additional logic for traps, e.g. Arena Trap, Fire Spin, etc.
                move_options[i].swap[j-1] = true;
            }
        }
    }
}
int applyModifier(const int val, const int numerator, const int denominator) {
  // Note the integer division
  // Technically should be uint32_t operations but I think it's fine here
  const int modifier = 4096 * numerator / denominator;
  return (val * modifier + 2048 - 1) / 4096;
}
int get_effective_attack_stat(const Pokemon& attacker, const Move &move) {
  int stat;
  // Physical/Special moves should all have Power! (I checked this)
  if(move.kind == MoveKind::PHYSICAL) {
    stat = attacker.stats.att;
  } else if(move.kind == MoveKind::SPECIAL) {
    stat = attacker.stats.spatt;
  } else {
    // ERROR
    stat = -1;
  }
  // More move-specific logic?
  return stat;
}
int get_effective_defense_stat(const Pokemon& defender, const Move &move) {
  int stat;
  // Physical/Special moves should all have Power! (I checked this)
  if(move.kind == MoveKind::PHYSICAL) {
    stat = defender.stats.def;
  } else if(move.kind == MoveKind::SPECIAL) {
    stat = defender.stats.spdef;
  } else {
    // ERROR
    stat = -1;
  }
  // More move-specific logic?
  return stat;
}
int get_deterministic_dmg_roll(const Pokemon& attacker, const Pokemon& defender, const Move &move, bool crit, int roll_val) {
  int basePower = *(move.power); // TODO: implement callbacks <- needs chainModify
  // Crit RNG
  // Stats
  int attack = get_effective_attack_stat(attacker, move);
  int defense = get_effective_defense_stat(defender, move);
  int level = attacker.lvl;
  // TODO: Boosts, including potentially ignoring boosts
  // TODO: figure out CalculateStat and modify stats
  int baseDamage = (2 * level / 5 + 2) * basePower * attack / defense / 50;
  // From here: modifyDamage
  baseDamage += 2;
  // TODO: Spread (for single battles, Parental Bond)
  // TODO: Weather
  // Crit (chance: 1/16, multiplier: 1.5)
  if(crit) {
    baseDamage = applyModifier(baseDamage, 3, 2);
  }
  // Randomization (16 rolls)
  baseDamage = applyModifier(baseDamage, roll_val+85, 100);
  // STAB (TODO: check database for ??? type)
  int stab_numer = 1, stab_denom = 1;
  // isSTAB = move.forceSTAB || pokemon.hasType(type) || pokemon.getTypes(false, true).includes(type);
  if(attacker.has_type(move.type)) {
    // STAB multiplier is 1.5
    stab_numer = 3, stab_denom = 2;
  }
  // TODO: check ModifySTAB handler?
  baseDamage = applyModifier(baseDamage, stab_numer, stab_denom);
  // Type effectiveness: runEffectiveness is b/c Arceus/Silvally and Tera
  std::optional<int> typeMod{0};
  for(Type t : defender.types) {
    if(t != Type::END) {
      const int newTypeMod = get_type_effectiveness(move.type, t);
      if(newTypeMod < -1) {
        typeMod = std::nullopt;
      } else if(typeMod != std::nullopt) {
        *typeMod += newTypeMod;
      }
    }
  }
  if(typeMod == std::nullopt) {
    return 0;
  } else if(*typeMod > 0) {
    // std::cout << "Super effective!\n";
    for(;*typeMod>0; (*typeMod)--) {
      baseDamage *= 2;
    }
  } else if (*typeMod < 0) {
    // std::cout << "Not very effective!\n";
    for(;*typeMod<0; (*typeMod)++) {
      baseDamage /= 2;
    }
  }
  // Guts modifier
  // ModifyDamage event: items, flash fire. Ignore phases (old gen)
  // Min damage check
  if(baseDamage == 0) return 1;
  // 16-bit truncation?
  return baseDamage % (1<<16);
}
}