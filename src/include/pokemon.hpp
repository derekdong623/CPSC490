#pragma once

#include "ref.hpp"
#include <algorithm>
#include <map>
#include <string>
#include <vector>
namespace pkmn {
struct MoveSlot {
  MoveId id = MoveId::NONE;
  int pp = 0;
  int maxpp = 0;
  bool used = false; // For LastResort, updated in deductPP
};
// For comparisons, ACCURACY acts as None
struct NatureVal {
  ModifierId plus = ModifierId::ACCURACY;
  ModifierId minus = ModifierId::ACCURACY;
};
/*
What to call to initialize a Pokemon:
- The contructor Pokemon(name, lvl, nature, IVs, EVs)
- set_ability()
- add_move()...at least once

Optional for initialization:
- set_item()
*/
struct Pokemon {
  bool lock = false;
  // Basic data: you choose!
  PokeName name = PokeName::NONE;  // Forme, e.g. Mega or alternate form
  Species species = Species::NONE; // ~Pokedex number
  Nature nature = Nature::NONE;
  int lvl = 0;
  Gender gender = Gender::NONE;  // Should be set to one valid for species
  std::array<MoveSlot, 4> moves; // Won't do move-changing/Transform
  Item item = Item::NO_ITEM;
  // Filled in from choices like above.
  Stats init_stats{}, stats{}; // Transform changes stat values
  int current_hp = 0;
  std::array<Type, 2> types;
  Ability baseAbility, ability;
  int effectiveSpeed = 0; // Dependent on e.g. boosts/TrickRoom
  // Related to Battle
  bool fainted = false;
  bool faintQueued = false;
  int side = -1;             // For BattleState
  int position = -1;         // For dragging in
  int activeMoveActions = 0; // For FakeOut etc.
  bool isActive = false;     // For BattleState
  int activeTurns = 0;       // For Slow Start, Speed Boost, etc.
  bool isStarted = false;    // For onStart callbacks I think?
  // Adjusted in state changes
  ModifierTable boosts;
  Status status = Status::NO_STATUS;
  // Trapping status
  bool trapped = false;
  /* Volatiles */
  // Very volatile (one turn only)
  bool switchFlag = false;      // TODO: Go through all involved logic
  bool forceSwitchFlag = false; // For dragging out
  bool newlySwitched = false;   // For e.g. FakeOut
  bool beingCalledBack = false; // For Pursuit
  bool flinch = false;
  bool endure = false;
  bool skipBeforeSwitchOutEventFlag = false; // For BatonPass/ShedTail
  bool roosted = false;
  bool weatherSuppressEnding = false; // For AirLock/CloudNine
  bool abilitySuppressEnding = false; // For NeutralizingGas
  bool beakBlasting = false;          // For BeakBlast midturn charge
  bool sparklingAria = false;         // For SparklingAria
  bool wasHurtThisTurn = false;       // For Assurance
  // Item volatiles
  Item lastItem = Item::NO_ITEM;
  bool itemKnockedOff = false;
  bool usedItemThisTurn = false;
  bool ateBerry = false;
  bool flungThisTurn = false;
  bool unburden = false;
  // Condition volatiles
  int confusion = 0;
  int toxicStage = 0;
  int lockedMove = 0;
  int stallTurns = 0;   // Set to 2 when starting a stall move
  int stallCounter = 1; // Probability of success is 1/n
  int stockpileTurns = 0;
  bool trappedCondition = false; // Note distinct from trapped!
  bool attracted = false;
  int taunted = 0;
  bool foresight = false;
  bool miracleeye = false;
  bool minimized = false;
  int telekinesised = 0;
  bool smackeddown = false;
  bool ingrained = false;
  int magnetrise = 0; // MagnetRise
  bool noretreat = false;
  bool berryWeakened = false; // For Ripen extra weakening
  bool charging = false;      // For the move "Charge"
  // Two-turn move volatiles
  // Set to each int to 2 when starting
  bool twoTurnMove = false;
  int inAir = 0;       // Bounce, Fly
  int underground = 0; // Dig
  int underwater = 0;  // Dive
  int inShadow = 0;    // PhantomForce, ShadowForce
  // For StompingTantrum and TemperFlare
  bool moveThisTurnFailed = false;
  bool moveLastTurnFailed = false;
  // Specific-moves volatiles
  bool statsRaisedThisTurn = false;  // BurningJealousy
  bool statsLoweredThisTurn = false; // LashOut
  MoveId lastMove = MoveId::NONE;    // Metronome/Instruct/MetalPowder?

  // INITIALIZATION
  Stats get_stats(const Stats &base, const Stats &nature, const Stats &IVs, const Stats &EVs,
                  const int lvl, const bool is_shedinja);
  Pokemon() {}
  Pokemon(PokeName, int lvl_, Gender, const Nature, const Stats &IVs, const Stats &EVs);
  bool operator==(const Pokemon &other) const {
    return position >= 0 && other.position >= 0 && side == other.side && position == other.position;
  }
  // Returns whether or not a non-trivial move was overwritten
  bool add_move(MoveId id, int slot);
  void set_ability(Ability ability_);
  void set_item(Item item_);
  inline void lockPokemon() { lock = true; };
  inline bool has_ability(Ability ability_) const { return ability == ability_; }
  inline bool has_item(Item item_) const { return item == item_; }
  inline bool has_type(Type t) const { return types[0] == t || types[1] == t; }
  inline bool isOnlyType(Type t) const { return types[0] == t && types[1] == Type::NO_TYPE; }
  void set_type(std::vector<Type> toTypes);
  bool has_volatile(VolatileId);
  void clearVolatile(bool clearSwitchFlags);
  // Apply a change of boostVal to a boostable stat.
  int boostStat(ModifierId, int boostVal); // Remember to reset on switches!
  // Get raw "base" stat.
  int getStatVal(ModifierId statName) const;
  // Get stat with (un)boost(s) applied.
  // I think statName is only ever in ATTACK, SPATT, DEFENSE, or SPDEF.
  int calculateStat(ModifierId statName, int boostIndex, Pokemon const &statUser) const;
  // Get stat with/without boost and/or onModifyStat applied.
  // statName should only be in ATTACK, SPATT, DEFENSE, SPDEF, or SPD.
  int getStat(ModifierId statName, bool boosted, bool modified) const;
  NatureVal getNature();
  bool isSemiInvulnerable();
  bool boost(ModifierTable &, EffectKind);
  bool useItem(bool eat, bool forceEat);
  std::optional<Item> takeItem(Pokemon &taker);
  int heal(int damage, EffectKind effectKind);
  int applyHeal(int damage);
  void addConfusion(bool axeKick);
  void tryTrap();
  int deductPP(MoveSlot &);
  void decrVolDurations();

  /* CALLBACKS */

  bool applyOnTryImmunity(Pokemon &user, MoveId);
  void applyOnChangeBoost(ModifierTable &, EffectKind);
  void applyOnTryBoost(ModifierTable &, EffectKind);
  void applyOnAfterEachBoost(int numBoost, EffectKind effectKind);
  void applyOnAfterBoost(ModifierTable &, EffectKind);
  void applyOnEat(Item);
  void applyOnEatItem(Item);
  void applyOnAfterUseItem();
  bool applyOnTakeItem(Item, Pokemon &taker);
  int applyOnTryHeal(int damage, EffectKind);
  bool applyOnTryEatItem();
};
} // namespace pkmn