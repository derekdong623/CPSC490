#include "pokemon.hpp"
#include "utils.hpp"

#include <optional>
namespace pkmn {
bool has_flinching_move(Pokemon *pkmn) {
  static const std::vector<MoveId> flinchingMoves = {
      MoveId::AIRSLASH,  MoveId::ASTONISH,       MoveId::BITE,        MoveId::BONECLUB,
      MoveId::DARKPULSE, MoveId::DOUBLEIRONBASH, MoveId::DRAGONRUSH,  MoveId::EXTRASENSORY,
      MoveId::FAKEOUT,   MoveId::FIERYWRATH,     MoveId::HEADBUTT,    MoveId::HEARTSTAMP,
      MoveId::HYPERFANG, MoveId::ICICLECRASH,    MoveId::IRONHEAD,    MoveId::MOUNTAINGALE,
      MoveId::NEEDLEARM, MoveId::ROCKSLIDE,      MoveId::ROLLINGKICK, MoveId::SKYATTACK,
      MoveId::SNORE,     MoveId::STEAMROLLER,    MoveId::STOMP,       MoveId::TWISTER,
      MoveId::UPPERHAND, MoveId::WATERFALL,      MoveId::ZENHEADBUTT, MoveId::ZINGZAP,
  };
  for (int i = 0; i < 4; i++) {
    if (std::find(flinchingMoves.begin(), flinchingMoves.end(), pkmn->moves[i].id) !=
        flinchingMoves.end()) {
      return true;
    }
  }
  return false;
}
Pokemon::Pokemon(PokeName name_, int lvl_, Gender gender_, const Nature nature_, const Stats &IVs,
                 const Stats &EVs)
    : name(name_), species(pokedex.species_dict[name_]), nature(nature_), lvl(lvl_),
      gender(gender_) {
  init_stats = get_stats(pokedex.base_stat_dict[name_], get_nature_contrib(nature), IVs, EVs, lvl_,
                         name_ == PokeName::SHEDINJA); // COMPUTING STATS
  stats = init_stats;                                  // Copy initial stats to current
  current_hp = init_stats.hp;
  auto tp = pokedex.type_dict[name_]; // shorthand
  types[0] = tp.first, types[1] = tp.second;
  hasFlinchingMove = has_flinching_move(this);
  clearVolatile(true);
}

// COMPUTING STATS
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

// slot must be in [0,4)
bool Pokemon::add_move(MoveId id, int slot) {
  if (lock) {
    std::cerr << "[Pokemon::add_move] Tried adding move to locked Pokemon." << std::endl;
    return false;
  }
  bool ret = (moves[slot].id != MoveId::NONE);
  moves[slot].id = id;
  moves[slot].pp = moves[slot].maxpp = moveDict.dict[id].pp;
  return ret;
}
void Pokemon::set_ability(Ability ability_) {
  if (lock) {
    std::cerr << "[Pokemon::set_ability] Tried changing ability of locked Pokemon." << std::endl;
    return;
  }
  baseAbility = ability = ability_;
}
void Pokemon::set_item(Item item_) {
  if (lock) {
    // std::cerr << "[Pokemon::set_item] Tried changing item of locked Pokemon." << std::endl;
    return;
  }
  item = item_;
}

// types should be length 1 or 2
void Pokemon::set_type(std::vector<Type> toTypes) {
  types[0] = toTypes[0];
  types[1] = toTypes.size() > 1 ? toTypes[1] : Type::NO_TYPE;
}
bool Pokemon::has_move(MoveId move) {
  for (int i = 0; i < 4; i++) {
    if (moves[i].id == move)
      return true;
  }
  return false;
}
// TODO: finish, or at least track progress
bool Pokemon::has_volatile(VolatileId vol) {
  switch (vol) {
  case VolatileId::FLINCH: {
    return flinch;
  }
  case VolatileId::CONFUSION: {
    return confusion;
  }
  case VolatileId::TWOTURN_MOVE: {
    return twoTurnMove;
  }
  case VolatileId::ATTRACT: {
    return attracted;
  }
  case VolatileId::ROOST: {
    return roosted;
  }
  case VolatileId::UNBURDEN: {
    return unburden;
  }
  case VolatileId::SPARKLINGARIA: {
    return sparklingAria;
  }
  case VolatileId::LEECHSEED: {
    return leechSeed;
  }
  default:
    return false;
  }
}
void Pokemon::clearVolatile(bool clearSwitchFlags) {
  boosts = {};
  // Copy moveSlots?

  // transformed = false;
  ability = baseAbility;
  // hpType = baseHpType;
  // hpPower = baseHpPower;
  // Remove linked volatiles and volatiles
  if (clearSwitchFlags) {
    switchFlag = false;
    forceSwitchFlag = false;
  }

  lastMove = MoveId::NONE;
  // lastMoveUsed = null;
  // moveThisTurn = '';
  moveThisTurnFailed = false;
  moveLastTurnFailed = false;

  // lastDamage = 0;
  // attackedBy = [];
  wasHurtThisTurn = false;
  newlySwitched = true;
  beingCalledBack = false;

  // volatileStaleness = undefined;

  // Clear condition volatiles
  confusion = 0;
  toxicStage = 0;
  lockedMove = 0;
  stallTurns = 0;
  stallCounter = 1;
  stockpileTurns = 0;
  attracted = false;
  taunted = 0;
  foresight = false;
  miracleeye = false;
  telekinesised = 0;
  smackeddown = false;
  ingrained = false;
  magnetrise = 0;
  // noretreat = false;
  berryWeakened = false;
  charging = false;
  // Clear item volatiles
  flungThisTurn = false;
  unburden = false;
  // Clear ability volatiles
  // Clear move volatiles
  roosted = false;
  beakBlasting = false;
  sparklingAria = false;
  twoTurnMove = false;
  inAir = 0;
  underground = 0;
  underwater = 0;
  inShadow = 0;
  // Clear other volatiles
  trapped = false;
  trappedCondition = false;
  flinch = false;
  endure = false;

  // species = baseSpecies // Do later
}
// Raw form of boosting stats
// Returns delta in stat applied
int Pokemon::boostStat(ModifierId stat, int boostVal) {
  int oldStatVal = boosts[stat];
  boosts[stat] = std::min(6, std::max(-6, boosts[stat] + boostVal));
  return boosts[stat] - oldStatVal;
}

int Pokemon::getStatVal(ModifierId statName) const {
  switch (statName) {
  case ModifierId::ATTACK:
    return stats.att;
  case ModifierId::SPATT:
    return stats.spatt;
  case ModifierId::DEFENSE:
    return stats.def;
  case ModifierId::SPDEF:
    return stats.spdef;
  default: // SPD
    return stats.spd;
  }
}
int Pokemon::calculateStat(ModifierId statName, int boostIndex, Pokemon const &statUser) const {
  int statVal = getStatVal(statName);
  // TODO: Wonder Room, use this.getStat()
  // TODO: onModifyBoost, uses Pokemon const &statUser || this
  return boostStatVal(statVal, boostIndex);
}
int Pokemon::getStat(ModifierId statName, bool boosted, bool modified) const {
  int statVal = getStatVal(statName);
  // TODO: Wonder Room special case
  if (boosted) {
    // TODO: onModifyBoost
    if(boosts.find(statName) != boosts.end())
      statVal = boostStatVal(statVal, boosts.at(statName));
  }
  if (modified) {
    // TODO: onModifyStat for each stat
    // NB: I think only ever used for spe?
    if (statName == ModifierId::SPEED) {
      statVal = applyOnModifySpeed(statVal);
    }
  }
  // Can probably skip truncation
  return statVal;
}
// Get the boosted and dropped stats from the nature, if applicable.
NatureVal Pokemon::getNature() {
  Stats vals = get_nature_contrib(nature);
  NatureVal ret;
  if (vals.att > 0)
    ret.plus = ModifierId::ATTACK;
  else if (vals.att < 0)
    ret.minus = ModifierId::ATTACK;
  if (vals.def > 0)
    ret.plus = ModifierId::DEFENSE;
  else if (vals.def < 0)
    ret.minus = ModifierId::DEFENSE;
  if (vals.spatt > 0)
    ret.plus = ModifierId::SPATT;
  else if (vals.spatt < 0)
    ret.minus = ModifierId::SPATT;
  if (vals.spdef > 0)
    ret.plus = ModifierId::SPDEF;
  else if (vals.spdef < 0)
    ret.minus = ModifierId::SPDEF;
  if (vals.spd > 0)
    ret.plus = ModifierId::SPEED;
  else if (vals.spd < 0)
    ret.minus = ModifierId::SPEED;
  return ret;
}
bool Pokemon::isSemiInvulnerable() {
  // Do later:
  // if(isSkyDropped()) return true;
  return inAir || underground || underwater || inShadow;
}
// Apply (un)boost(s) to a target.
// Returns false with failure; if no issue but no boosting, still returns true.
// NB: With callbacks onChangeBoost(), onTryBoost(), onAfterEachBoost(), onAfterBoost(),
// and of course bound to [-6, 6].
bool Pokemon::boost(ModifierTable &boostTable, EffectKind effectKind, bool isSelf) {
  if (!current_hp)
    return false;
  if (!isActive)
    return false;
  // // Q: I don't think it's strictly necessary? Doesn't seem to change outcome.
  // if(!teams[1-side].pokemonLeft) return;
  applyOnChangeBoost(boostTable, effectKind);
  applyOnTryBoost(boostTable, effectKind, isSelf);
  for (auto &[mod, b] : boostTable) {
    int boostBy = boostStat(mod, b);
    if (boostBy)
      applyOnAfterEachBoost(b, effectKind, isSelf);
  }
  applyOnAfterBoost(boostTable, effectKind);
  for (auto &[mod, b] : boostTable) {
    if (b > 0) {
      statsRaisedThisTurn = true;
    } else if (b < 0) {
      statsLoweredThisTurn = true;
    }
  }
  return true;
}
ModifierTable getItemBoosts(Item item) {
  switch (item) {
  case Item::CELL_BATTERY:
  case Item::SNOWBALL: {
    return {{ModifierId::ATTACK, 1}};
  }
  case Item::ELECTRIC_SEED:
  case Item::GRASSY_SEED: {
    return {{ModifierId::DEFENSE, 1}};
  }
  case Item::ABSORB_BULB:
  case Item::THROAT_SPRAY: {
    return {{ModifierId::SPATT, 1}};
  }
  case Item::LUMINOUS_MOSS:
  case Item::MISTY_SEED:
  case Item::PSYCHIC_SEED: {
    return {{ModifierId::SPDEF, 1}};
  }
  case Item::ADRENALINE_ORB: {
    return {{ModifierId::SPEED, 1}};
  }
  case Item::ROOM_SERVICE: {
    return {{ModifierId::SPEED, -1}};
  }
  case Item::WEAKNESS_POLICY: {
    return {{ModifierId::ATTACK, 2}, {ModifierId::SPATT, 2}};
  }
  default:
    break;
  }
  return {};
}
bool Pokemon::useItem(bool eat, bool forceEat) {
  if (!isActive)
    return false;
  if (!current_hp) {
    if (!isGem(item) || (item != Item::JABOCA_BERRY && item != Item::ROWAP_BERRY)) {
      return false;
    }
  }
  if (item == Item::NO_ITEM || itemKnockedOff)
    return false;
  // TODO: figure out sourceEffect stuff
  // "if an item is telling us to eat it but we aren't holding it, we probably shouldn't eat what we
  // are holding"
  // onUseItem seems to do nothing
  // item boosts
  ModifierTable boostTable = getItemBoosts(item);
  boost(boostTable, EffectKind::NO_EFFECT, true);
  bool succEat = eat && (forceEat || applyOnTryEatItem());
  if (succEat) {
    applyOnEat(item);
    applyOnEatItem(item);
    ateBerry = true;
  }
  // MirrorHerb only: Gen9
  // else if (!eat) {
  //   applyOnUse();
  // }
  if (succEat || !eat) {
    lastItem = item;
    item = Item::NO_ITEM;
    // battle.clearEffectState(itemState);
    usedItemThisTurn = true;
    // TODO: onAfterUseItem(): Unburden
    applyOnAfterUseItem();
    return true;
  }
  return false;
}
// Returns nullopt for failure
std::optional<Item> Pokemon::takeItem(Pokemon &taker) {
  if (item == Item::NO_ITEM || itemKnockedOff) {
    return std::nullopt;
  }
  if (applyOnTakeItem(item, taker)) {
    Item oldItem = item;
    item = Item::NO_ITEM;
    // TODO: clear target.itemState
    // onEnd(): MirrorHerb N/A, UtilityUmbrella
    if (oldItem == Item::UTILITY_UMBRELLA) {
      // Do later: weatherchange
    }
    // N/A onAfterTakeItem()
    return oldItem;
  }
  return std::nullopt;
}
// Actually apply the healing (capped by max HP)
// PokemonShowdown Pokemon.heal()
// Returns -1 for fail
int Pokemon::applyHeal(int damage) {
  if (damage <= 0)
    return 0;
  if (current_hp <= 0)
    return -1;
  if (!isActive)
    return -1;
  if (current_hp >= stats.hp)
    return -1;
  current_hp += damage;
  if (current_hp > stats.hp) {
    damage -= current_hp - stats.hp;
    current_hp = stats.hp;
  }
  return damage;
}
// Handles the larger-scale idea of healing a Pokemon: called often
// PokemonShowdown Battle.heal()
// Returns actual damage healed, or -1 for failure
int Pokemon::heal(int damage, EffectKind effectKind) {
  damage = applyOnTryHeal(damage, effectKind);
  if (!damage)
    return damage;
  if (!current_hp || !isActive)
    return 0;
  int healAmt = applyHeal(damage);
  // applyOnHeal(healAmt, target, source, effect); // Does nothing it seems
  return healAmt;
}
void Pokemon::addConfusion(bool axeKick) {
  // Confusion has no restart
  if (!confusion) {
    int minTurns = axeKick ? 3 : 2;
    confusion = math::random(minTurns, 6);
  }
}
void Pokemon::tryTrap() {
  // Remark: runStatusImmunity just checks typing and onImmunity
  if (!has_type(Type::GHOST)) { // Only type that's immune
    trapped = true;
  }
}
// Deduct a single PP from the move slot
int Pokemon::deductPP(MoveSlot &moveSlot) {
  if (!moveSlot.pp)
    return 0;
  moveSlot.pp--;
  return 1;
}
// Decrements durations of valid volatiles
// - Confusion uses time not duration
void Pokemon::decrVolDurations() {
  if (flinch)
    flinch = false;
}
} // namespace pkmn