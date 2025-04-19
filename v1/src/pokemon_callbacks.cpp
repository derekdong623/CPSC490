#include "pokemon.hpp"
#include "utils.hpp"

#include <optional>
namespace pkmn {
// false for immune, true for not-immune
bool Pokemon::applyOnTryImmunity(Pokemon &user, MoveId move) {
  switch (move) {
  case MoveId::ATTRACT:
  case MoveId::CAPTIVATE: {
    // Specifically opposite-gender
    return (gender == Gender::FEMALE && user.gender == Gender::MALE) ||
           (gender == Gender::MALE && user.gender == Gender::FEMALE);
  }
  case MoveId::DREAMEATER: {
    return status == Status::SLEEP || ability == Ability::COMATOSE;
  }
  case MoveId::ENDEAVOR: {
    return user.current_hp < current_hp;
  }
  case MoveId::LEECHSEED: {
    return !has_type(Type::GRASS);
  }
  case MoveId::OCTOLOCK: {
    // The only natural immunity to trapped is Ghost-type
    return !has_type(Type::GHOST);
  }
  case MoveId::SWITCHEROO:
  case MoveId::TRICK: {
    return ability != Ability::STICKY_HOLD;
  }
  case MoveId::WORRYSEED: {
    // From PokemonShowdown:
    // Truant and Insomnia have special treatment; they fail before
    // checking accuracy and will double Stomping Tantrum's BP
    return ability != Ability::TRUANT && ability != Ability::INSOMNIA;
  }
  case MoveId::SYNCHRONOISE: {
    for (auto typ : user.types) {
      if (typ != Type::NO_TYPE && has_type(typ)) {
        return true;
      }
    }
    return false;
  }
  default:
    break;
  }
  return true;
}
// Just Contrary, Ripen, Simple.
void Pokemon::applyOnChangeBoost(std::map<ModifierId, int> &boostTable, EffectKind effectKind) {
  switch (ability) {
  case Ability::CONTRARY: {
    for (auto &[mod, b] : boostTable) {
      b *= -1;
    }
    break;
  }
  case Ability::RIPEN: {
    if (effectKind == EffectKind::BERRY) {
      for (auto &[mod, b] : boostTable) {
        b *= 2;
      }
    }
    break;
  }
  case Ability::SIMPLE: {
    for (auto &[mod, b] : boostTable) {
      b *= 2;
    }
    break;
  }
  default:
    break;
  }
}
void Pokemon::applyOnTryBoost(std::map<ModifierId, int> &boostTable, EffectKind effectKind) {
  auto stopFoeLowering = [&boostTable]() {
    // TODO: If source is self, don't change boost
    for (auto it = boostTable.begin(); it != boostTable.end();) {
      if (it->second < 0) {
        it = boostTable.erase(it); // erase returns the next iterator
      } else {
        ++it;
      }
    }
  };
  switch (ability) {
  // Prevents others from lowering its defense
  case Ability::BIG_PECKS: {
    // TODO: If source is self, don't change boost
    if (boostTable.contains(ModifierId::DEFENSE) && boostTable[ModifierId::DEFENSE] < 0) {
      boostTable.erase(ModifierId::DEFENSE);
    }
    break;
  }
  // Prevents others from lowering its attack
  case Ability::HYPER_CUTTER: {
    // TODO: If source is self, don't change boost
    if (boostTable.contains(ModifierId::ATTACK) && boostTable[ModifierId::ATTACK] < 0) {
      boostTable.erase(ModifierId::ATTACK);
    }
    break;
  }
  // Prevents others from lowering its accuracy
  case Ability::ILLUMINATE:
  case Ability::KEEN_EYE:
  case Ability::MINDS_EYE: {
    // TODO: If source is self, don't change boost
    if (boostTable.contains(ModifierId::ACCURACY) && boostTable[ModifierId::ACCURACY] < 0) {
      boostTable.erase(ModifierId::ACCURACY);
    }
    break;
  }
  // Prevents others from lowering *any* stat
  case Ability::CLEAR_BODY:
  case Ability::WHITE_SMOKE:
  case Ability::FULL_METAL_BODY: { // Note: FullMetalBody isn't breakable!
    stopFoeLowering();
    break;
  }
  // Prevents Intimidate
  case Ability::INNER_FOCUS:
  case Ability::OBLIVIOUS:
  case Ability::OWN_TEMPO:
  case Ability::SCRAPPY: {
    if (effectKind == EffectKind::INTIMIDATE && boostTable.contains(ModifierId::ATTACK)) {
      boostTable.erase(ModifierId::ATTACK);
    }
  }
    // case Ability::MIRROR_ARMOR:

  default:
    break;
  }
  if (item == Item::CLEAR_AMULET) {
    stopFoeLowering();
  }
  // TODO: Dang...I need the field (Mist)
}
void Pokemon::applyOnAfterEachBoost(int numBoost, EffectKind effectKind) {
  if (ability == Ability::COMPETITIVE && effectKind != EffectKind::STICKY_WEB) {
    // TODO: Check that there's a non-self source
    if (numBoost < 0) {
      boost({{ModifierId::SPATT, 2}}, EffectKind::NO_EFFECT);
    }
  }
  if (ability == Ability::DEFIANT && effectKind != EffectKind::STICKY_WEB) {
    // TODO: Check that there's a non-self source
    if (numBoost < 0) {
      boost({{ModifierId::ATTACK, 2}}, EffectKind::NO_EFFECT);
    }
  }
}
void Pokemon::applyOnAfterBoost(std::map<ModifierId, int> &boostTable, EffectKind effectKind) {
  if (ability == Ability::RATTLED) {
    if (effectKind == EffectKind::INTIMIDATE && boostTable.contains(ModifierId::ATTACK)) {
      boost({{ModifierId::SPEED, 1}}, EffectKind::NO_EFFECT);
    }
  }
  if (item == Item::ADRENALINE_ORB) {
    // From PokemonShowdown:
    // Adrenaline Orb activates if Intimidate is blocked by an ability like Hyper Cutter,
    // which deletes boost.atk, but not if the holder's attack is already at -6 (or +6 if
    // it has Contrary), which sets boost.atk to 0.
    if (boosts.spd < 6 &&
        (!boostTable.contains(ModifierId::ATTACK) || boostTable[ModifierId::ATTACK] != 0) &&
        effectKind == EffectKind::INTIMIDATE) {
      useItem(false, false);
    }
  }
  // TODO: EjectPack
}
// Apply effect(s) from having eaten the berry.
void Pokemon::applyOnEat() {
  static std::map<Item, ModifierId> badMinusNatures = {
      {Item::FIGY_BERRY, ModifierId::ATTACK}, {Item::IAPAPA_BERRY, ModifierId::DEFENSE},
      {Item::AGUAV_BERRY, ModifierId::SPEED}, {Item::MAGO_BERRY, ModifierId::SPEED},
      {Item::WIKI_BERRY, ModifierId::SPATT},
  };
  switch (item) {
  case Item::AGUAV_BERRY:
  case Item::FIGY_BERRY:
  case Item::IAPAPA_BERRY:
  case Item::MAGO_BERRY:
  case Item::WIKI_BERRY: {
    applyHeal(stats.hp / 3);
    if (getNature().minus == badMinusNatures[item]) {
      addConfusion(false);
    }
    break;
  }
  case Item::LIECHI_BERRY: {
    boostStat(ModifierId::ATTACK, 1);
    break;
  }
  case Item::GANLON_BERRY:
  case Item::KEE_BERRY: {
    boostStat(ModifierId::DEFENSE, 1);
    break;
  }
  case Item::PETAYA_BERRY: {
    boostStat(ModifierId::SPATT, 1);
    break;
  }
  case Item::APICOT_BERRY:
  case Item::MARANGA_BERRY:
  case Item::SALAC_BERRY: {
    boostStat(ModifierId::SPEED, 1);
    break;
  }
  case Item::LANSAT_BERRY: {
    // addVolatile(VolatileId::FOCUS_ENERGY);
    break;
  }
  case Item::LUM_BERRY: {
    status = Status::NO_STATUS;
    confusion = 0;
    break;
  }
  case Item::PERSIM_BERRY: {
    confusion = 0;
    break;
  }
  case Item::ORAN_BERRY: {
    applyHeal(10);
    break;
  }
  case Item::SITRUS_BERRY: {
    applyHeal(stats.hp / 4);
    break;
  }
  // Boost a random non-maxed regular stat by 2
  case Item::STARF_BERRY: {
    std::vector<ModifierId> boostable;
    if (boosts.att < 6)
      boostable.push_back(ModifierId::ATTACK);
    if (boosts.def < 6)
      boostable.push_back(ModifierId::DEFENSE);
    if (boosts.spatt < 6)
      boostable.push_back(ModifierId::SPATT);
    if (boosts.spdef < 6)
      boostable.push_back(ModifierId::SPDEF);
    if (boosts.spd < 6)
      boostable.push_back(ModifierId::SPEED);
    if (!boostable.empty()) {
      boostStat(boostable[math::random(static_cast<int>(boostable.size()))], 2);
    }
    break;
  }
  case Item::LEPPA_BERRY: {
    int slot = -1;
    for (int i = 0; i < 4; i++) {
      if (moves[i].id != MoveId::NONE && moves[i].pp == 0) {
        slot = i;
        break;
      }
    }
    if (slot < 0) {
      for (int i = 0; i < 4; i++) {
        if (moves[i].id != MoveId::NONE && moves[i].pp < moves[i].maxpp) {
          slot = i;
          break;
        }
      }
    }
    if (slot >= 0) {
      moves[slot].pp += 10;
      moves[slot].pp = std::min(moves[slot].maxpp, moves[slot].pp);
    }
    break;
  }
  // Q: Should all of these statuses be asserts instead of ifs?
  case Item::ASPEAR_BERRY: {
    if (status == Status::FREEZE)
      status = Status::NO_STATUS;
    break;
  }
  case Item::CHERI_BERRY: {
    if (status == Status::PARALYSIS)
      status = Status::NO_STATUS;
    break;
  }
  case Item::CHESTO_BERRY: {
    if (status == Status::SLEEP)
      status = Status::NO_STATUS;
    break;
  }
  case Item::PECHA_BERRY: {
    if (status == Status::POISON || status == Status::TOXIC)
      status = Status::NO_STATUS;
    break;
  }
  case Item::RAWST_BERRY: {
    if (status == Status::BURN)
      status = Status::NO_STATUS;
    break;
  }
    // TODO: MicleBerry
  default:
    break;
  }
}
// Just CheekPouch and Ripen; CudChew is Gen9
void Pokemon::applyOnEatItem() {
  switch (ability) {
  case Ability::CHEEK_POUCH: {
    applyHeal(stats.hp / 3);
    break;
  }
  case Ability::RIPEN: {
    static const std::vector<Item> weakenBerries = {
        Item::BABIRI_BERRY, Item::CHARTI_BERRY, Item::CHILAN_BERRY, Item::CHOPLE_BERRY,
        Item::COBA_BERRY,   Item::COLBUR_BERRY, Item::HABAN_BERRY,  Item::KASIB_BERRY,
        Item::KEBIA_BERRY,  Item::OCCA_BERRY,   Item::PASSHO_BERRY, Item::PAYAPA_BERRY,
        Item::RINDO_BERRY,  Item::ROSELI_BERRY, Item::SHUCA_BERRY,  Item::TANGA_BERRY,
        Item::WACAN_BERRY,  Item::YACHE_BERRY,
    };
    berryWeakened =
        std::find(weakenBerries.begin(), weakenBerries.end(), item) != weakenBerries.end();
    break;
  }
  default:
    break;
  }
}
// Just Unburden
void Pokemon::applyOnAfterUseItem() {
  if (ability == Ability::UNBURDEN) {
    // What does checking pokemon != effectState.target do?
    unburden = true;
    // addVolatile(VolatileId::UNBURDEN);
  }
}
// False if blocked by !checkedBerserk or HealBlock.
bool Pokemon::applyOnTryEatItem() {
  // TODO: Berserk
  switch (item) {
  case Item::AGUAV_BERRY:
  case Item::FIGY_BERRY:
  case Item::IAPAPA_BERRY:
  case Item::MAGO_BERRY:
  case Item::WIKI_BERRY: {
    if (!applyOnTryHeal(stats.hp / 3, EffectKind::BERRY))
      return false;
    break;
  }
  case Item::ENIGMA_BERRY:
  case Item::SITRUS_BERRY: {
    if (!applyOnTryHeal(stats.hp / 4, EffectKind::BERRY))
      return false;
    break;
  }
  case Item::ORAN_BERRY: {
    if (!applyOnTryHeal(10, EffectKind::BERRY))
      return false;
    break;
  }
  default:
    break;
  }
  return true;
}
// Potential modifiers to heal amount
// Called by oheal(...) and applyOnTryEatItem() (healing berries).
// Essentially returns false if HealBlock applies.
int Pokemon::applyOnTryHeal(int damage, EffectKind effectKind) {
  // BigRoot: -Drain, LeechSeed, Ingrain, AquaRing, StrengthSap
  if (has_item(Item::BIG_ROOT) && effectKind == EffectKind::MOVE) {
    damage = applyModifier(damage, 5324, 4096); // Interesting it's not 5325/4096
  }
  // Ripen
  if (has_ability(Ability::RIPEN) && effectKind == EffectKind::BERRY) {
    damage *= 2;
  }
  // HealBlock: complicated
  return damage;
}
} // namespace pkmn