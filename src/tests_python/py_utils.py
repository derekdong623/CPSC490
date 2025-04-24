import sys
import os

# Add the compiled module path to sys.path
sys.path.append(
    os.path.abspath(os.path.join(os.path.dirname(__file__), "../build/bin"))
)
from fast_pkmn import *

def log_pkmn(pkmn: Pokemon, prefix: str=''):
    print(f"{prefix}{pkmn.name}")
    print(f"{prefix}HP: {pkmn.hp}")
    print(f"{prefix}item: {str(pkmn.item)}")
    print(f"{prefix}{pkmn.status}")
    boost_str = [prefix]
    for m, b in pkmn.boosts.items():
        if b != 0:
            boost_str.append(f"{str(m)},{b}")
    print(f"{prefix}Boosts: {';'.join(boost_str)}")


def get_choice_str(state: BattleState, side: int):
    team = state.teams[side]
    pkmn = state.get_active(side)
    choice = team.choices
    assert sum(choice.move) + sum(choice.swap) == 1
    moveInd = choice.move.index(True) if True in choice.move else -1
    swapInd = choice.swap.index(True) if True in choice.swap else -1
    ret = ['']
    if choice.mega:
        ret.append('[mega]')
    if moveInd >= 0:
        ret.append(f'[move:{moveInd}:{pkmn.moves[moveInd].id}]')
    if swapInd >= 0:
        ret.append(f'[swap:{swapInd}:{team.pkmn[swapInd].name}]')
    return ' '.join(ret)

def log_state(state: BattleState, turn_num: int=None):
    if turn_num is not None:
        print(f"Starting turn {turn_num}...\n")
    # Print active Pokemon HP and status
    print("Agent active:")
    log_pkmn(state.get_active(0), '\t')
    print("Opp active:")
    log_pkmn(state.get_active(1), '\t')
    print("Agent team:")
    print(f"\t{' '.join(f'{str(pkmn.name)}:HP{pkmn.hp}:POS{pkmn.pos}' for pkmn in state.teams[0].pkmn)}")
    print("Opp team:")
    print(f"\t{' '.join(f'{str(pkmn.name)}:HP{pkmn.hp}:POS{pkmn.pos}' for pkmn in state.teams[1].pkmn)}")
    print()