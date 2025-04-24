import sys
import os

# Add the compiled module path to sys.path
sys.path.append(
    os.path.abspath(os.path.join(os.path.dirname(__file__), "../build/bin"))
)
from fast_pkmn import *
from py_utils import *
import random

ALL_MODIFIER_IDS = [
    ModifierId.ATTACK,
    ModifierId.DEFENSE,
    ModifierId.SPATT,
    ModifierId.SPDEF,
    ModifierId.SPEED,
    ModifierId.ACCURACY,
    ModifierId.EVASION,
]


class RandomAgent:
    def get_choice(self, state: BattleState, side: int, choices):
        return random.choice(choices)


class PokemonGame:
    """Creates a game using the given Pokemon teams."""

    def __init__(self, agent_team, opp_team, use_opp=False):
        self._ai_agent = RandomAgent()
        self._teams = [agent_team, opp_team]
        self._use_opp = use_opp

    def initial_state(self):
        state = BattleState()
        for side in range(2):
            state.set_team(side, self._teams[side])
        state.start_battle()
        return state

    def payoff(self, state: BattleState):
        """Determines if the given state is a win for the agent given the game is over."""
        ret = 0
        win_val = state.check_win()
        if win_val < 0:
            return 0
        elif win_val == 0:
            ret += 1000
        else:
            ret -= 5000
        ret += state.teams[0].pokemon_left * 500
        return ret

    def game_over(self, state: BattleState):
        return state.check_win() >= 0

    def get_choice_list(self, state: BattleState, side: int):
        # Returned choices should be hashable
        # Tuple[bool, Tuple(4), Tuple(4)]
        ch = state.teams[side].choices
        ret = []
        for i, m in enumerate(ch.move):
            if m:
                ret.append(
                    (
                        False,
                        tuple(j == i for j in range(4)),
                        tuple(False for j in range(6)),
                    )
                )
        for i, s in enumerate(ch.swap):
            if s:
                ret.append(
                    (
                        False,
                        tuple(False for j in range(4)),
                        tuple(j == i for j in range(6)),
                    )
                )
        return ret

    def default_get_Choice(self, state: BattleState, side: int):
        """Returns a Choice"""
        cl = self.get_choice_list(state, side)
        return random.choice(cl)

    def to_Choice(self, choice):
        # Turns a hashable choice into a Choice
        move = choice[1].index(True) if True in choice[1] else -1
        swap = choice[2].index(True) if True in choice[2] else -1
        return Choice(choice[0], move, swap)

    def get_successor(self, state: BattleState, choice, verbose: bool=False):
        state.verbose = verbose
        state.teams[0].choices = self.to_Choice(choice)
        if self._use_opp:
          state.fill_opp(1)
        else:
          state.teams[1].choices = self.to_Choice(
              self._ai_agent.get_choice(state, 1, self.get_choice_list(state, 1))
          )
        if verbose:
            print(f"Agent choice: {get_choice_str(state, 0)}")
            print(f"Opp choice: {get_choice_str(state, 1)}")
            print()
        state = state.run_turn()
        # print('running turn')
        if self.game_over(state):
            return state
        # print('still alive')
        while not self.game_over(state) and state.is_instaswitch():
            if verbose:
                log_state(state)
            if state.teams[0].instaswitch:
                state.teams[0].choices = self.to_Choice(
                    self.default_get_Choice(state, 0)
                )
                if verbose:
                    print(f"Agent choice (instaswitch): {get_choice_str(state, 0)}")
                # print('still alive')
            if state.teams[1].instaswitch:
                if self._use_opp:
                  state.fill_opp(1)
                # print('still alive')
                else:
                  state.teams[1].choices = self.to_Choice(
                      self.default_get_Choice(state, 1)
                  )
                if verbose:
                  print(f"Opp choice (instaswitch): {get_choice_str(state, 1)}")
            if verbose:
                print()
            # print('running turn')
            state = state.run_turn()
            # print('still alive')
        if verbose:
            log_state(state)
        state.verbose = False
        return state

    def simplify_state(self, state: BattleState):
        # State: Tuple[Team]
        # Team: Tuple[Tuple[Pokemon], activeInd: int]
        # Pokemon: Tuple[HP, Item, Boosts, Tuple[Move]]
        # Move: pp: int
        def to_hashable(boost_dict):
            ret = tuple(
                boost_dict[mod] if mod in boost_dict else 0 for mod in ALL_MODIFIER_IDS
            )
            return ret
        ret = tuple(
            (
                tuple(
                    (
                        pkmn.hp,
                        pkmn.item,
                        to_hashable(pkmn.boosts),
                        tuple(move.pp for move in pkmn.moves),
                    )
                    for pkmn in state.teams[side].pkmn
                ),
                state.teams[side].active,
            )
            for side in range(2)
        )
        return ret
