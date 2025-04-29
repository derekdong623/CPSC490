import random
import sys
import mcts

# import argparse
import time
import math
import datetime
import json
import argparse
from pokemon_game import PokemonGame

import os

# Add the compiled module path to sys.path
sys.path.append(
    os.path.abspath(os.path.join(os.path.dirname(__file__), "../build/bin"))
)

from fast_pkmn import *
from py_utils import *


NUM_GAMES = 500
TIME_LIMIT = 1


def simulate_one(game: PokemonGame, time_limit: float, verbose: bool = False):
    """
    verbose: Print each choice made and the resulting state.
    Return: (score, Pokemon left, average rollouts, num turns, max time used)
    """
    p1_time = 0.0
    # start with a fresh copy of the policy function
    p1_mcts = mcts.MCTS(time_limit, game)
    state = game.initial_state()

    total_rollouts = 0
    turn_num = 0

    while not game.game_over(state):
        turn_num += 1
        start = time.time()
        choice, nr = p1_mcts.mcts_search(state)
        p1_time = max(p1_time, time.time() - start)
        total_rollouts += nr
        # print("Number of rollouts:", nr)
        if verbose:
            log_state(state, turn_num)
        # print(state.get_active(0).hp, state.get_active(1).hp, choice)
        state = game.get_successor(state, choice, verbose)
    if verbose:
        print(f"Payoff: {game.payoff(state)}")
    return (
        game.payoff(state),
        state.teams[0].pokemon_left,
        total_rollouts / turn_num,
        turn_num,
        p1_time,
    )


def simulations_results(
    game: PokemonGame, num_games: int, time_limit: float, report_every: int = None
):
    def update(mean, var, val, n):
        delta = val - mean
        mean += delta / n
        var += delta * (val - mean)
        return mean, var

    if num_games == 1:
        simulate_one(game, time_limit, True)
        return

    p1_wins, p1_win_var = 0, 0
    p1_score, p1_var = 0, 0
    num_win = 0
    p1_time = 0.0

    for i in range(num_games):
        if report_every is not None and (i + 1) % report_every == 0:
            print(f"Starting game: {i+1}, Current winrate: {p1_wins:.3f}")
        score, pkmn_left, avg_rollout, num_turns, t = simulate_one(game, time_limit)
        print(f"Avg_rollouts {avg_rollout} Turns {num_turns}")
        p1_time = max(p1_time, t)
        p1_score, p1_var = update(p1_score, p1_var, score, i + 1)
        if score == 0:
            # Should never happen given my implementation?
            p1_wins, p1_win_var = update(p1_wins, p1_win_var, 0.5, i + 1)
        elif score > 0:
            num_win += 1
            print("Pokemon left:", pkmn_left)
            p1_wins, p1_win_var = update(p1_wins, p1_win_var, 1, i + 1)
        else:
            print("Loss")
            p1_wins, p1_win_var = update(p1_wins, p1_win_var, 0, i + 1)

    if p1_time > time_limit * 1.01:
        print("WARNING: max time for agent =", p1_time)
    margin, margin_var = p1_score, p1_var / (num_games - 1)
    wins, win_var = p1_wins, p1_win_var / (num_games - 1)
    print(
        f"NET: {margin:.2f} +/- {2 * math.sqrt(margin_var / NUM_GAMES):.2f}; WINS: {wins:.3f} +/- {2 * math.sqrt(win_var / NUM_GAMES):.3f}"
    )


def get_pkmn(dic, item_name: str = None, use_iv: bool = True):
    ivs = Stats(31, 31, 31, 31, 31, 31)
    if use_iv and "ivs" in dic:
        ivs = Stats(*dic["ivs"])
    pkmn = Pokemon(
        getattr(PokeName, dic["name"].upper()),
        dic["lvl"],
        getattr(Gender, dic["gender"].upper()),
        getattr(Nature, dic["nature"].upper()),
        ivs,
        Stats(),
    )
    pkmn.set_ability(getattr(Ability, dic["ability"].upper()))
    if item_name is not None:
        pkmn.set_item(getattr(Item, item_name))
    elif "item" in dic:
        pkmn.set_item(getattr(Item, dic["item"].upper()))
    assert len(dic["moves"]) <= 4, "too many moves!"
    for i, move in enumerate(dic["moves"]):
        pkmn.add_move(getattr(MoveId, move.upper()), i)
    return pkmn


def read_team(filename: str, item_name: str = None, use_iv: bool = True):
    ret = Team()
    with open(filename) as file:
        dic = json.load(file)
        for i, x in enumerate(dic):
            ret.add_pokemon(i, get_pkmn(x, item_name, use_iv))
        return ret


if __name__ == "__main__":
    print(f"Start time: {datetime.datetime.now()}")
    parser = argparse.ArgumentParser()
    parser.add_argument("--agent", type=str, required=True)
    parser.add_argument("--opp", type=str, required=True)
    parser.add_argument("--games", type=int, default=500)
    parser.add_argument("--time", type=float, required=True)
    parser.add_argument("--report-every", type=int, dest="report")
    # parser.add_argument("--verbose", "-v", action="store_true")
    parser.add_argument("--item", type=str)
    parser.add_argument("--iv", action="store_true")
    args = parser.parse_args()
    TEAMS_DIR = "src/tests_python/teams"
    agent_team = read_team(
        os.path.join(TEAMS_DIR, f"agent_teams/{args.agent}.json"),
        args.item,
        not args.iv,
    )
    opp_team = read_team(os.path.join(TEAMS_DIR, f"{args.opp}.json"))
    game = PokemonGame(agent_team, opp_team, use_opp=True)
    simulations_results(
        game, args.games, args.time, report_every=args.report
    )
