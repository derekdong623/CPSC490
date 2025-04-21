import random
import sys
import mcts

# import argparse
import time
import math
import datetime
from pokemon_game import PokemonGame

import os

# Add the compiled module path to sys.path
sys.path.append(
    os.path.abspath(os.path.join(os.path.dirname(__file__), "../build/bin"))
)

from fast_pkmn import *


def get_charmander(lvl=5):
    charmander = Pokemon(
        PokeName.CHARMANDER,
        lvl,
        Gender.MALE,
        Nature.HARDY,
        Stats(31, 31, 31, 31, 31, 31),
        Stats(),
    )
    charmander.set_ability(Ability.BLAZE)
    charmander.add_move(MoveId.SCRATCH, 0)
    charmander.add_move(MoveId.EMBER, 1)
    charmander.add_move(MoveId.WILLOWISP, 2)
    return charmander


def get_bulbasaur(lvl=5):
    bulbasaur = Pokemon(
        PokeName.BULBASAUR,
        lvl,
        Gender.MALE,
        Nature.HARDY,
        Stats(31, 31, 31, 31, 31, 31),
        Stats(),
    )
    bulbasaur.set_ability(Ability.OVERGROW)
    bulbasaur.add_move(MoveId.TACKLE, 0)
    return bulbasaur

def get_piplup(lvl=5, item=Item.NO_ITEM):
    piplup = Pokemon(
        PokeName.PIPLUP,
        lvl,
        Gender.MALE,
        Nature.BASHFUL,
        Stats(31, 31, 31, 31, 31, 31),
        Stats(),
    )
    piplup.set_ability(Ability.DEFIANT)
    piplup.add_move(MoveId.POUND, 0)
    piplup.add_move(MoveId.GROWL, 1)
    piplup.add_move(MoveId.BUBBLE, 2)
    piplup.add_move(MoveId.PLUCK, 3)
    return piplup

def get_poochyena(lvl=5, item=Item.NO_ITEM):
    poochyena = Pokemon(
        PokeName.POOCHYENA,
        lvl,
        Gender.MALE,
        Nature.BASHFUL,
        Stats(31, 31, 31, 31, 31, 31),
        Stats(),
    )
    poochyena.set_ability(Ability.RATTLED)
    poochyena.add_move(MoveId.BITE, 0)
    poochyena.add_move(MoveId.QUICKATTACK, 1)
    poochyena.add_move(MoveId.SANDATTACK, 2)
    return poochyena

def get_lillipup(lvl=5):
    lillipup = Pokemon(
        PokeName.LILLIPUP,
        lvl,
        Gender.MALE,
        Nature.BASHFUL,
        Stats(31, 31, 31, 31, 31, 31),
        Stats(),
    )
    lillipup.set_ability(Ability.VITAL_SPIRIT)
    lillipup.add_move(MoveId.TACKLE, 0)
    lillipup.add_move(MoveId.BITE, 1)
    lillipup.add_move(MoveId.SANDATTACK, 2)
    return lillipup

def get_rookidee(lvl=5):
    rookidee = Pokemon(
        PokeName.ROOKIDEE,
        lvl,
        Gender.MALE,
        Nature.BASHFUL,
        Stats(31, 31, 31, 31, 31, 31),
        Stats(),
    )
    rookidee.set_ability(Ability.KEEN_EYE)
    rookidee.add_move(MoveId.WINGATTACK, 0)
    rookidee.add_move(MoveId.SANDATTACK, 1)
    rookidee.add_move(MoveId.SWAGGER, 2)
    return rookidee



NUM_GAMES = 500
TIME_LIMIT = 0.1


def simulations_results(game: PokemonGame, num_games, time_limit_1, report_every=None):
    p1_wins = 0
    p1_win_var = 0
    p2_wins = 0
    p1_score = 0
    p1_var = 0
    p1_time = 0.0
    num_win, num_tie, num_loss = 0, 0, 0

    for i in range(num_games):
        if report_every is not None and (i+1) % report_every == 0:
            print("Starting game:", i+1)
        # start with a fresh copy of the policy function
        p1_mcts = mcts.MCTS(time_limit_1, game)
        state = game.initial_state()

        while not game.game_over(state):
            start = time.time()
            choice = p1_mcts.mcts_search(state)
            # print(state.get_active(0).hp, state.get_active(1).hp, choice)
            p1_time = max(p1_time, time.time() - start)
            state = game.get_successor(state, choice)

        def update(mean, var, val, n):
            delta = val - mean
            mean += delta / n
            var += delta * (val - mean)
            return mean, var

        score = game.payoff(state)
        # print(score)
        p1_score, p1_var = update(p1_score, p1_var, score, i + 1)
        if game.payoff(state) == 0:
            # Should never happen given my implementation?
            num_tie += 1
            p1_wins, p1_win_var = update(p1_wins, p1_win_var, 0.5, i + 1)
            p2_wins += 0.5
        elif score > 0:
            num_win += 1
            p1_wins, p1_win_var = update(p1_wins, p1_win_var, 1, i + 1)
        else:
            num_loss += 1
            p1_wins, p1_win_var = update(p1_wins, p1_win_var, 0, i + 1)
            p2_wins += 1

    # print(num_tie, num_win, num_loss)
    if p1_time > time_limit_1 + 0.01:
        print("WARNING: max time for agent =", p1_time)
    return p1_score, p1_var / (num_games - 1), p1_wins, p1_win_var / (num_games - 1)


if __name__ == "__main__":
    print(f"Start time: {datetime.datetime.now()}")
    agent_team = Team()
    # Maybe implement Pluck?
    agent_team.add_pokemon(0, get_piplup(6, Item.ORAN_BERRY))
    agent_team.add_pokemon(1, get_poochyena(6, Item.ORAN_BERRY))
    opp_team = Team()
    # Bite: flinch, QuickAttack: priority, SandAttack: accuracy drop
    opp_team.add_pokemon(0, get_poochyena(5))
    opp_team.add_pokemon(1, get_lillipup(6))
    # Swagger: confusion, attack boost
    opp_team.add_pokemon(2, get_rookidee(6))
    game = PokemonGame(agent_team, opp_team)
    margin, margin_var, wins, win_var = simulations_results(
        game,
        NUM_GAMES,
        TIME_LIMIT,
        report_every=10
    )
    print(
        f"NET: {margin:.2f} +/- {2 * math.sqrt(margin_var / NUM_GAMES):.2f}; WINS: {wins:.2f} +/- {2 * math.sqrt(win_var / NUM_GAMES):.2f}"
    )
