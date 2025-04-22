import sys
import os
from time import time

# Add the compiled module path to sys.path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "../build/bin")))

from fast_pkmn import *

def get_charmander(lvl=5):
  charmander = Pokemon(PokeName.CHARMANDER, lvl, Gender.MALE, Nature.HARDY, Stats(31, 31, 31, 31, 31, 31), Stats())
  charmander.set_ability(Ability.BLAZE)
  charmander.add_move(MoveId.SCRATCH, 0)
  charmander.add_move(MoveId.EMBER, 1)
  charmander.add_move(MoveId.WILLOWISP, 2)
  return charmander

def get_bulbasaur(lvl=5):
  bulbasaur = Pokemon(PokeName.BULBASAUR, lvl, Gender.MALE, Nature.HARDY, Stats(31, 31, 31, 31, 31, 31), Stats())
  bulbasaur.set_ability(Ability.OVERGROW)
  bulbasaur.add_move(MoveId.TACKLE, 0)
  return bulbasaur

# test_options()
state = BattleState()
teams = [Team(), Team()]
teams[0].add_pokemon(0, get_charmander(5))
teams[0].add_pokemon(1, get_bulbasaur(5))
teams[1].add_pokemon(0, get_bulbasaur(100))
state.set_team(0, teams[0]), state.set_team(1, teams[1])
state.start_battle()
assert state.check_win() < 0
state.teams[0].choices = Choice(False, 0, -1)
state.teams[1].choices = Choice(False, 0, -1)
start_time = time()
state = state.run_turn()
print("Time for running turn:", time() - start_time)
assert state.is_instaswitch()
state.teams[0].choices = Choice(False, -1, 1)
start_time = time()
state = state.run_turn()
print("Time for running instaswitch etc.:", time() - start_time)
assert not state.is_instaswitch()

# test ember
state = BattleState()
teams = [Team(), Team()]
teams[0].add_pokemon(0, get_charmander(5))
teams[1].add_pokemon(0, get_bulbasaur(5))
state.set_team(0, teams[0]), state.set_team(1, teams[1])
state.start_battle()
state.teams[0].choices = Choice(False, 1, -1)
state.teams[1].choices = Choice(False, 0, -1)
print(state.get_active(1).hp)
start_time = time()
state = state.run_turn()
print("Time for running turn:", time() - start_time)
assert not state.is_instaswitch()
print(state.get_active(1).hp)