import random
import util
from util import Stat, Type, MoveCategory\

class PokemonGame:
    ''' Creates a game using the given Pokemon teams.'''
    def __init__(
            self, 
            teams, 
            ai_team_name, 
            agent_team_name, 
        ):
        self._teams = (teams[ai_team_name], teams[agent_team_name])
        self._ai_agent = PokemonAI(self)

    def new_state(self, state, param_name, pkmn, delta, is_agent=True):
        state = [list(x) for x in state]
        val = state[is_agent][self.get_param_index(param_name)] + delta
        if param_name == 'Hp':
            state[is_agent][self.get_param_index(param_name)] = min(val, pkmn[is_agent].stats[Stat.Hp])
        else: # |Base stat stages| capped at 6
            state[is_agent][self.get_param_index(param_name)] = min(6, max(-6, val))
        return tuple([tuple(x) for x in state])


    def initial_position(self):
        ''' Returns initial state based on base stats of given teams. '''
        # TEMPORARY: state == (ai: (index, hp, stat_stages), agent: (index, hp, stat_stages))
        return tuple([
            tuple([0, self._teams[i][0].stats[Stat.Hp]] + [0] * 6) 
            for i in range(2)
        ])
    

    def moves(self, state, is_agent=True):
        ''' Returns the list of moves the agent can take at state. '''
        # TEMPORARY: no switching, only select move
        return self._teams[is_agent][self.get_param(state, "active_pokemon", is_agent)].moves
    
    def get_multiplier(self, state, param_name, is_agent):
        stages = self.get_param(state, param_name, is_agent)
        return [2/8, 2/7, 2/6, 2/5, 2/4, 2/3, 1, 1.5, 2, 2.5, 3, 3.5, 4][stages + 6]
    

    def get_pkmn(self, state):
        return [self._teams[i][self.get_param(state, "active_pokemon", False)] for i in range(2)]
    

    def get_damage(self, state, pkmn, move, is_agent):
        if move.category == MoveCategory.Status:
            return 0
        attacker = pkmn[is_agent]
        defender = pkmn[not is_agent]
        # Base power
        dmg = move.power
        # Overgrow/Blaze/Torrent
        if self.get_param(state, "Hp", is_agent) <= attacker.stats[Stat.Hp] and \
            (
                attacker.ability == "Overgrow" and move.type == Type.Grass or 
                attacker.ability == "Blaze" and move.type == Type.Fire or
                attacker.ability == "Torrent" and move.type == Type.Water
            ):
            dmg = int(dmg * 1.5)

        # Start calculateBaseDamageSMSSSV
        # Base damage
        # int(int(int(0.4 * level + 2) * basePower * attack / defense) / 50 + 2)
        dmg = int(0.4 * attacker.level + 2) * move.power
        attackStat = Stat.Attack if move.category == MoveCategory.Physical else Stat.Special_Attack
        attackParam = f"{attackStat.name}_stage"
        attack = int(attacker.stats[attackStat] * self.get_multiplier(state, attackParam, is_agent))
        defenseStat = Stat.Defense if move.category == MoveCategory.Physical else Stat.Special_Defense
        defenseParam = f"{defenseStat.name}_stage"
        defense = int(defender.stats[defenseStat] * self.get_multiplier(state, defenseParam, is_agent))
        dmg = int(int(dmg * attack / defense) / 50 + 2)


        

        # Crit
        # NB: For this project, can be disabled
        if not self._disable_random_factors:
            if random.randrange(16) < 1: # RnB rules: 1/16 chance
                dmg = int(dmg * 1.5) # RnB rules: 1.5x mult
        # End calculateBaseDamageSMSSSV

        # Damage roll
        dmg = int(dmg * ((85 + random.randrange(16)) / 100))

        # STAB
        if move.type in attacker.types:
            dmg = int(dmg * 1.5)
        # Effectiveness
        for defender_type in defender.types:
            dmg *= self._typechart[move.type][defender_type]
        dmg = int(dmg)
        # TODO: Burn
        return dmg
    

    def get_speed(self, state, pkmn, is_agent):
        return int(pkmn[is_agent].stats[Stat.Speed] * self.get_multiplier(state, "Speed_stage", is_agent))
    

    def result(self, state, agent_move, ai_move=None):
        ''' Returns the state obtained by both players making their move. '''
        # TEMPORARY: no switching, assume both moves are Moves
        if ai_move is None:
            ai_move = self._ai_agent.policy(state)
        moves_chosen = [ai_move, agent_move]
        pkmn = self.get_pkmn(state)

        # Figure out who moves first
        adjusted_speeds = [self.get_speed(state, pkmn, bool(i)) for i in range(2)]
        agent_turn = False
        # TODO: priority
        if adjusted_speeds[1] > adjusted_speeds[0]:
            agent_turn = True
        # Actual speed-tie implementation is Yates shuffle -- more important for
        # multi-battles and reproducibility IMO, result is equal-weighted chance
        elif random.random() < 0.5: 
            agent_turn = True
        
        flinch = False
        for _ in range(2):
            if flinch or self.game_over(state):
                continue
            move = moves_chosen[agent_turn]

            # Damage
            dmg = self.get_damage(state, pkmn, move, is_agent=agent_turn)
            state = self.new_state(state, 'Hp', pkmn, -dmg, not agent_turn)


            # TODO: accuracy/evasiveness, PP, contact, status

            # Stat changes
            if pkmn[agent_turn].ability != "Clear Body":
                pass # TODO: self-stat change moves
            if pkmn[not agent_turn].ability != "Clear Body":
                for stat, delta in move.stat['Target'].items():
                    state = self.new_state(state, f'{stat.name}_stage', delta, not agent_turn)

            # Flinch
            # NB: For this project, can be disabled
            if not self._disable_random_factors:
                if pkmn[not agent_turn].ability != 'Inner Focus' and move.name in ['Bite']:
                    if random.random() < 0.3:
                        flinch = True
            
            # TODO: Recovery for other moves
            if move.name == 'Absorb':
                healing = max(1, int(dmg * 0.5))
                state = self.new_state(state, 'Hp', pkmn, healing, agent_turn)

            agent_turn = not agent_turn
        return state


    
    def game_over(self, state):
        ''' Determines if the given state represents the end of a battle. '''
        # I think I just expose a C++ check?
        return state.winner() >= 0


    def payoff(self, state):
        ''' Determines if the given state is a win for the agent given the game is over. '''
        # state is a BattleState
        # I think I just expose a C++ check?
        return [1, -1][state.winner()]

    
    def simulate(self, policy, n, report_every=None):
        ''' 
        Simulates games using the given policy and returns the
        winning percentage for the policy.

        policy -- a function from states to moves
        '''
        self._simulation = True
        wins = 0
        for game_num in range(n):
            if report_every is not None and game_num % report_every == 0:
                print(f"Starting game index {game_num}...")
            state = self.initial_position()
            while not self.game_over(state):
                ai_move = self._ai_agent.policy(state)
                agent_move = policy(state)
                state = self.result(state, agent_move, ai_move)
            if self.payoff(state) > 0:
                wins += 1
        return wins / n

class PokemonAI:
    def __init__(self, game: PokemonGame):
        self._game = game

    def policy(self, state):
        model = self._game
        moves = model.moves(state, False)
        scores = [0] * len(moves)
        pkmn = model.get_pkmn(state)
        # Highest damaging move/killing move
        dmgs = [(model.get_damage(state, pkmn, m, False), i) for i, m in enumerate(moves)]
        dmgs.sort()
        for dmg, ind in dmgs:
            if dmg >= model.get_param(state, "Hp"):
                # Fast kill
                ai_spd, agent_spd = model.get_speed(state, pkmn, False), model.get_speed(state, pkmn, True)
                if ai_spd >= agent_spd: # Speed ties count as fast
                    scores[ind] += 6
                else:
                    scores[ind] += 3
                if random.random() < 0.8:
                    scores[ind] += 6
                else:
                    scores[ind] += 8
                
            elif dmg == dmgs[-1][0]:
                if random.random() < 0.8:
                    scores[ind] += 6
                else:
                    scores[ind] += 8
        scores = [(score, ind) for ind, score in enumerate(scores)]
        scores.sort()
        possible = []
        for score, ind in scores:
            if score == scores[-1][0]:
                possible.append(ind)
        move_chosen = moves[random.choice(possible)]
        return move_chosen