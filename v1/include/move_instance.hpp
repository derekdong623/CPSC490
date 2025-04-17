#pragma once
#include "pokemon.hpp"
#include <queue>
namespace pkmn {
// Stores data on the running of a move
struct MoveInstance {
  Move moveData;
  MoveId id;
  int priority = 0;
  int totalDamage = 0;
  int selfHealed = 0;
  bool crit = false;
  bool flinch = false;
  int numHits = 1;
  bool parentalBond = false;
  bool hasBounced = false;
  bool hasSheerForce = false; // Flag set in onModifyMove
  MoveInstance(Move &move) : id(move.id) { moveData = move; }
  bool breaksProtect();
  bool isMultiHit();
  bool isNoParentalBond();
  std::pair<int, int> getRecoil();
  bool isRecoil() { return getRecoil().first != 0; }
  bool isCharge();
  bool isFuture();
  bool isCalling();
  bool isOHKO();
  bool isSleepUsable();
  bool onModifyType(Pokemon const &pokemon);
  bool onModifyMove(Pokemon const &pokemon);
  int getNumHits(Pokemon const &pokemon);
  bool multiaccCheck(Pokemon const &target, Pokemon const &pokemon);
  bool getBasicAcc(Pokemon const &target, Pokemon const &pokemon, bool multiacc);
  bool alwaysSelfDestruct() {
    return id == MoveId::EXPLOSION || id == MoveId::SELFDESTRUCT || id == MoveId::MISTYEXPLOSION;
  }
};
} // namespace pkmn