#pragma once
#include "pokemon.hpp"
#include "utils.hpp"
#include <queue>
namespace pkmn {
// Stores data on the running of a move
struct MoveInstance {
  Move moveData;
  MoveId id=MoveId::NONE;
  int priority = 0;
  int totalDamage = 0;
  int selfHealed = 0;
  bool crit = false;
  bool flinch = false;
  bool selfDropped = false; // For selfDrops() == applySelfEffect() potential recursion ig
  int currentHitNum = 0;
  int numHits = 1; // For some onBasePower()
  bool parentalBond = false;
  bool hasBounced = false;
  bool hasSheerForce = false; // Flag set in onModifyMove
  bool infiltrates = false;   // Flag set in onModifyMove
  MoveInstance(Move &move) : id(move.id) { moveData = move; }
  // This constructor should ONLY be used for testing!
  MoveInstance(MoveId &moveId) : id(moveId) { moveData = moveDict.dict[moveId]; }
  bool breaksProtect();
  bool isMultiHit();
  bool isNoParentalBond();
  ModifierTable getBoosts();
  std::pair<int, int> getHeal();
  Status getStatus();
  std::pair<int, int> getRecoil();
  bool isRecoil() { return getRecoil().first != 0; }
  std::vector<SecondaryEffect> getSecondaries();
  SecondaryEffect getSelfEffect();
  bool isPowder();
  bool isCharge();
  bool isFuture();
  bool isCalling();
  bool isOHKO();
  bool isSleepUsable();
  bool isIffHitSelfDestruct();
  bool isSelfSwitch();
  bool forcesSwitch();
  bool makesContact(Pokemon &user);
  bool onModifyType(Pokemon const &pokemon);
  bool onModifyMove(Pokemon const &pokemon);
  int getNumHits(Pokemon const &pokemon);
  bool multiaccCheck(Pokemon const &user);
  int getBasicAcc(int acc, bool multiacc, Pokemon &target, Pokemon &pokemon);
  bool alwaysSelfDestruct() {
    return id == MoveId::EXPLOSION || id == MoveId::SELFDESTRUCT || id == MoveId::MISTYEXPLOSION;
  }
};
} // namespace pkmn