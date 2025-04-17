#include "battle_state.hpp"
namespace pkmn {
// Returns whether or not a Pokemon was overwritten.
// pokemon's species should not be NONE, as a basic check.
// Assumes pokemon is newly initialized, e.g. max HP.
bool Team::add_pokemon(int slot, Pokemon &pokemon) {
  if (pokemon.species == Species::NONE) {
    std::cerr << "[Team::add_pokemon] Tried adding un-initialized Pokemon to team." << std::endl;
    return false;
  }
  if (lock) {
    std::cerr << "[Team::add_pokemon] Tried adding Pokemon to locked team." << std::endl;
    return false;
  }
  bool overwritten = pkmn[slot].species != Species::NONE;
  if (!overwritten) {
    pokemonLeft++;
  }
  pkmn[slot] = pokemon;
  return overwritten;
}
// Prevents manual changes to Pokemon.
void Team::lockTeam() {
  if(lock) return;
  lock = true;
  for(int i=0; i<6; i++) {
    pkmn[i].lockPokemon();
  }
}
} // namespace pkmn