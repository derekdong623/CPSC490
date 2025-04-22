import sys
import os

# Add the compiled module path to sys.path
sys.path.append(
    os.path.abspath(os.path.join(os.path.dirname(__file__), "../build/bin"))
)
from pokemon_game import PokemonGame
from collections import defaultdict
import time
import math
import random


class Node:
    def __init__(self, state, parent=None):
        self.state = state
        self.parent = parent
        self.N = 0  # Total visits to this node
        self.Q = defaultdict(float)  # Q[action] = avg reward for action
        self.N_a = defaultdict(int)  # N_a[action] = count for action
        self.children = defaultdict(
            list
        )  # children[action] = list of Nodes (one per sampled outcome)


class MCTS:
    def __init__(self, allowed_sec, game: PokemonGame):
        self._allowed_sec = allowed_sec
        self._game: PokemonGame = game
        self._max_rollout_depth = 5

    def mcts_search(self, root_state):
        deadline = time.time() + self._allowed_sec
        root_node = Node(self._game.simplify_state(root_state))
        rolloutNum = 0
        while time.time() < deadline - 0.01:
            rolloutNum += 1
            state, node = root_state, root_node
            state.set_choices()
            path = []

            # SELECTION & EXPANSION
            turnNum = 0
            while not self._game.game_over(state):
                action = self.select_action(node, state)
                next_state = self._game.get_successor(state, action)
                next_state_simplified = self._game.simplify_state(next_state)

                # Check if this sampled next_state already exists under this action
                child_found = False
                for child in node.children[action]:
                    if child.state == next_state_simplified:
                        child_found = True
                        next_node = child
                        break

                if not child_found:
                    # Expand with new node for this sampled outcome
                    next_node = Node(next_state_simplified, parent=node)
                    node.children[action].append(next_node)

                path.append((node, action))
                state, node = next_state, next_node
                turnNum += 1

                if not child_found:
                    break  # Only expand one node per simulation
            reward = self.rollout(node, state)
            # BACKPROPAGATION
            for parent, action in reversed(path):
                parent.N += 1
                parent.N_a[action] += 1
                q = parent.Q[action]
                n = parent.N_a[action]
                parent.Q[action] += (reward - q) / n  # Incremental average
        # print("Num rollouts:", rolloutNum)
        return self.best_action(root_node)

    def select_action(self, node, state):
        c = 1.4
        total_N = node.N + 1e-6
        best_score = -float("inf")
        best_action = None
        for a in self._game.get_choice_list(state, 0):
            q = node.Q[a]
            n = node.N_a[a] + 1e-6
            ucb = q + c * (total_N / n) ** 0.5
            if ucb > best_score:
                best_score = ucb
                best_action = a
        return best_action

    def rollout(self, node, state):
        # Random rollout
        while not self._game.game_over(state):
            action = random.choice(self._game.get_choice_list(state, 0))
            state = self._game.get_successor(state, action)
        return self._game.payoff(state)

    def best_action(self, root_node):
        # Could choose highest-Q, but let's try visit count for now.
        return max(root_node.N_a.items(), key=lambda x: x[1])[0]
