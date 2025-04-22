import time
import random
import json
from collections import defaultdict
from pokemon_game import PokemonGame

# state == (ai_state, agent_state)
# Each state == (cur_pokemon, hp, stat_stages...)
# result() returns next state(probabilistically generated)

GAMMA = 0.9
EPSILON = 0.25
ALPHA_0 = 0.25
ALPHA_DECAY = 0.9999


def q_learn(model: PokemonGame, time_limit, filename=None):
    start_time = time.time()
    q_map = defaultdict(float)
    alpha_map = defaultdict(lambda:1.)
    
    def greedy_action(state, choices):
        return choices[max([(q_map[(state, choice)], i) for i, choice in enumerate(choices)])[1]]

    def policy(visible_state):
        # NB: returns action!
        # TODO: convert visible_state into an array of hashable choices that can be made
        return greedy_action(visible_state, choices)

    def choose_action(state):
        # NB: returns both!
        choices = model.get_choice_list(state)
        if random.random() < alpha_map[state] * EPSILON:  # Exploration: random move
        # if random.random() < EPSILON:  # Exploration: random move
            return random.choice(choices)
        else:  # Exploitation
            return greedy_action(state, choices)

    num_games = 0
    while time.time() - start_time < time_limit:
        pos = model.initial_position()      # Raw position as 4-tuple
        while not model.game_over(pos):
            S = pos
            a = choose_action(S)            # eps-chosen action from position
            new_pos = model.result(pos, a)
            S_prime = new_pos
            reward = 0
            if model.game_over(new_pos):
                reward = model.payoff(new_pos) * 1000
            moves = model.moves(S_prime, True)
            a_prime = greedy_action(S_prime, moves)
            q_map[(S, a.name)] += ALPHA_0 * alpha_map[S] * (
                reward + GAMMA * q_map[(S_prime, a_prime.name)] - q_map[(S, a.name)]
            )
            alpha_map[S] *= ALPHA_DECAY
            pos = new_pos
        num_games += 1
    
    if filename is not None:
        with open(f"{filename}_qmap.json", 'w') as file:
            json.dump({str(key): val for key, val in q_map.items()}, file)
        with open(f"{filename}_alpha.json", 'w') as file:
            json.dump({str(key): val for key, val in alpha_map.items()}, file)
    return num_games, policy
