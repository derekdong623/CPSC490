#include "battle_state.hpp"
namespace pkmn {
struct OppSwitchPriority {
  int pokeInd;
  int partyLoc;
  int score = 0;
  // We pop the *lower-priority* choices
  bool operator<(const OppSwitchPriority &other) const {
    std::tuple<int, int> thisTuple{-score, partyLoc};
    std::tuple<int, int> otherTuple{-other.score, other.partyLoc};
    return thisTuple < otherTuple;
  }
};
struct OppMovePriority {
  int moveSlotInd;
  MoveId id;
  int score = 0;
  // We pop the *lower-priority* choices
  bool operator<(const OppMovePriority &other) const { return score > other.score; }
};
// Account for multi-hit moves
int calc_dmg(int dmg_rolled, MoveId move, Pokemon &user) {
  int baseHits = baseNumHits(move);
  if (baseHits > 1) {
    if (baseHits == 5 && user.isActive && user.has_ability(Ability::SKILL_LINK)) {
      return dmg_rolled * 5;
    } else if (baseHits == 2) {
      return dmg_rolled * 2;
    }
    return dmg_rolled * 3;
  }
  return dmg_rolled;
}
// side: the side *with opponent AI* to fill in the choice for
void BattleState::fillOpponentChoice(int side) {
  Pokemon &pkmn = getActivePokemon(side);
  Pokemon &target = getActivePokemon(1 - side);
  if (teams[side].instaSwitch) {
    std::vector<OppSwitchPriority> switchChoices;
    for (int pokeInd = 0; pokeInd < 6; pokeInd++) {
      Pokemon &curPoke = teams[side].pkmn[pokeInd];
      if (curPoke.species != Species::NONE && !curPoke.fainted &&
          pokeInd != teams[side].activeInd) {
        switchChoices.push_back({pokeInd, curPoke.position});
      } else {
        continue;
      }
      int pkmnMaxDmgDealt = 0;
      for (int i = 0; i < 4; i++) {
        auto moveId = curPoke.moves[i].id;
        if (moveId != MoveId::NONE && moveDict.dict[moveId].category != MoveCategory::STATUS) {
          MoveInstance moveInst = MoveInstance(moveId);
          pkmnMaxDmgDealt = std::max(
              pkmnMaxDmgDealt,
              calc_dmg(
                  getDamage(curPoke, target, moveInst, {.roll_val = 15, .crit = 0}).damageDealt,
                  moveId, curPoke));
        }
      }
      int targetMaxDmgDealt = 0;
      for (int i = 0; i < 4; i++) {
        auto moveId = target.moves[i].id;
        if (moveId != MoveId::NONE && moveDict.dict[moveId].category != MoveCategory::STATUS) {
          MoveInstance moveInst = MoveInstance(moveId);
          targetMaxDmgDealt = std::max(
              targetMaxDmgDealt,
              calc_dmg(
                  getDamage(target, curPoke, moveInst, {.roll_val = 15, .crit = 0}).damageDealt,
                  moveId, target));
        }
      }
      if (pkmn.effectiveSpeed >= target.effectiveSpeed) {
        // Faster and OHKOs: +5
        if (pkmnMaxDmgDealt >= target.current_hp) {
          switchChoices.back().score = 5;
        }
        // Faster and deals more than taken (as % of current HP): +3
        else if (pkmnMaxDmgDealt * target.current_hp >= targetMaxDmgDealt * pkmn.current_hp) {
          switchChoices.back().score = 3;
        }
        // Only faster: +1
        else {
          switchChoices.back().score = 1;
        }
      } else {
        // Slower and OHKO'd: -1
        if (targetMaxDmgDealt >= pkmn.current_hp) {
          switchChoices.back().score = -1;
        }
        // Slower and deals more than taken (as % of current HP)
        // If OHKO but not OHKO'd: +4
        else if (pkmnMaxDmgDealt >= target.current_hp && targetMaxDmgDealt < pkmn.current_hp) {
          switchChoices.back().score = 4;
        }
        // Else: +2
        else if (pkmnMaxDmgDealt * target.current_hp >= targetMaxDmgDealt * pkmn.current_hp) {
          switchChoices.back().score = 2;
        }
        // Only slower: 0
        else {
          switchChoices.back().score = 0;
        }
      }
    }
    std::sort(switchChoices.begin(), switchChoices.end());
    // Select the first party location with the maximal score
    int chosenSlot = switchChoices[0].pokeInd;
    teams[side].choicesAvailable = Choice(false, -1, chosenSlot);
  } else {
    bool isKOed = false;
    // Check if active is KOed
    for (int i = 0; i < 4; i++) {
      auto id = target.moves[i].id;
      if (id != MoveId::NONE && !pkmn.moves[i].disabled &&
          moveDict.dict[id].category != MoveCategory::STATUS) {
        MoveInstance moveInst = MoveInstance(id);
        if (checkHit(target, pkmn, moveInst, false) &&
            calc_dmg(getDamage(target, pkmn, moveInst, {.crit = 0}).damageDealt, id, pkmn) >=
                pkmn.current_hp) {
          isKOed = true;
        }
      }
    }
    std::vector<OppMovePriority> moveChoices;
    // Contains: damage, kill, index
    std::vector<std::tuple<int, bool, int>> dmgMoves;
    for (int i = 0; i < 4; i++) {
      auto id = pkmn.moves[i].id;
      if (id != MoveId::NONE && !pkmn.moves[i].disabled) {
        // Default score for non-attacking (status?) moves is +6
        if (moveDict.dict[id].category == MoveCategory::STATUS) {
          moveChoices.push_back({i, id, 6});
          // TODO: Check if move is "useless"
        } else {
          // Add to dmgMoves queue with roll:
          MoveInstance moveInst = MoveInstance(id);
          // e.g. shouldn't FakeOut after first turn
          if (checkHit(pkmn, target, moveInst, false)) {
            // Opponent AI only sees crit if a move always crits
            int dmg = std::max(
                0, calc_dmg(getDamage(pkmn, target, moveInst, {.crit = 0}).damageDealt, id, pkmn));
            dmgMoves.push_back({dmg, dmg > target.current_hp, i});
          }
        }
      }
    }
    std::sort(dmgMoves.begin(), dmgMoves.end());
    // Decreasing in dmg or kill
    std::reverse(dmgMoves.begin(), dmgMoves.end());
    if (!dmgMoves.empty()) {
      int highestDmg = std::get<0>(dmgMoves[0]);
      int highestRollScore = math::randomChance(80, 100) ? 6 : 8;
      if (std::get<1>(dmgMoves[0])) {
        highestRollScore += 3;
        // if (verbose) {
        //   std::cout << "Fast kill\n";
        // }
      }
      size_t numCurMoves = moveChoices.size();
      for (int i = 0; i < dmgMoves.size(); i++) {
        int moveSlotInd = std::get<2>(dmgMoves[i]);
        auto id = pkmn.moves[moveSlotInd].id;
        // Kills get highest-roll score boost
        if (std::get<0>(dmgMoves[i]) == highestDmg || std::get<1>(dmgMoves[i])) {
          moveChoices.push_back({moveSlotInd, id, highestRollScore});
          // Killing priority moves count as fast-kills
          if (pkmn.effectiveSpeed >= target.effectiveSpeed || moveDict.dict[id].priority > 0) {
            moveChoices.back().score += 3;
          }
        }
        // TODO: Moxie, Beast Boost, Chilling Neigh, or Grim Neigh score boost
        // TODO: high crit chance and super-effective score boost
        // special cases
        switch (id) {
        case MoveId::FAKEOUT: {
          if (!pkmn.activeMoveActions && !target.has_ability(Ability::SHIELD_DUST) &&
              !target.has_ability(Ability::INNER_FOCUS)) {
            // if (verbose) {
            //   std::cout << "Boosted fakeout\n";
            // }
            moveChoices.back().score += 9;
          }
          break;
        }
        case MoveId::THUNDERWAVE:
        case MoveId::STUNSPORE:
        case MoveId::GLARE:
        case MoveId::NUZZLE: {
          moveChoices.back().score += 7;
          // 50% no change, 50% -1
          if (math::randomChance(1, 2))
            moveChoices.back().score--;
          if ((pkmn.effectiveSpeed < target.effectiveSpeed &&
               pkmn.effectiveSpeed >= target.effectiveSpeed / 4) ||
              pkmn.has_move(MoveId::HEX) || pkmn.hasFlinchingMove || target.attracted ||
              target.confusion) {
            moveChoices.back().score++;
          }
          break;
        }
        default:
          break;
        }
        // slower and killed: priority gets +11
        if (isKOed && pkmn.effectiveSpeed < target.effectiveSpeed &&
            moveDict.dict[id].priority > 0) {
          moveChoices.back().score += 11;
        }
      }
    }
    std::sort(moveChoices.begin(), moveChoices.end());
    if (moveChoices.empty()) {
      std::cerr << "Opponent did not settle on any move!\n";
    }
    for (int i = moveChoices.size() - 1; i >= 0; i--) {
      if (moveChoices[i].score < moveChoices[0].score) {
        moveChoices.pop_back();
      } else {
        break;
      }
    }
    // Randomly select between the highest-scoring moves
    int chosenMove = moveChoices[math::random(moveChoices.size())].moveSlotInd;
    teams[side].choicesAvailable = Choice(false, chosenMove, -1);
  }
}
} // namespace pkmn