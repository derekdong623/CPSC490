import os
from pathlib import Path
import csv
import json
import re

START_DIR = Path(__file__).parent.parent.resolve()
# DIR = os.path.join(START_DIR, 'data/base_gen_8')
DIR = os.path.join(START_DIR, 'data/run_and_bun')

def read_base_stats():
    DIR = os.path.join(START_DIR, 'data/base_gen_8')
    # I downloaded Pokedex CSV data (dex number, type(s), base stats and for different forms) from PokemonDB
    # This parses it into space-delimited minimal lines to be read by the C programs
    # For now I separate "normal" forms from alternate forms -- this is probably wrong since e.g. Lycanroc
    # has no "normal" form, just several possible forms. I just wanted the Pokedex number to match the line...
    with open(os.path.join(DIR, 'base_stats.csv'), 'r') as file_stats:
        with open(os.path.join(DIR, 'base_stats_normal.txt'), 'w') as file_norm:
            with open(os.path.join(DIR, 'base_stats_form.txt'), 'w') as file_form:
                reader = csv.DictReader(file_stats)  # Reading as a list
                for row in reader:
                    res = ' '.join([row['HP'], row['Attack'], row['Defense'], row['Sp.Attack'], row['Sp.Defense'], row['Speed']])
                    if row['Form']:
                        file_form.write(f"{row['Form']} {res}\n")
                    else:
                        file_norm.write(f"{res}\n")
    # I also generate the list of species to be copy-pasted into pokemon_enums.hpp from this data
    list_of_species = []
    with open(os.path.join(DIR, 'base_stats.csv'), 'r') as file:
        reader = csv.DictReader(file)  # Reading as a list
        for row in reader:
            if not row['Form']:
                list_of_species.append(
                    row['Name'].upper()
                    .replace('. ', '_')
                    .replace(': ', '_')
                    .replace(' ', '_')
                    .replace('.', '')
                    .replace('-', '_')
                    .replace("'", ''))
    with open(os.path.join(DIR, 'pkmn_list.txt'), 'w') as file:
        # NOTE: manually replaced Flabebe with alphanumeric e's and Nidorans with _M, _F
        for species in list_of_species:
            file.write(f'{species},\n')

def read_move_data():
    DIR = os.path.join(START_DIR, 'data/base_gen_8')
    # I got data from the Moves database from PokemonDB. Unlike Pokedex, there's no convenient download option,
    # so I copy-pasted the table source code they have (for all-through-gen-9 moves since there's limited sorting)
    # and use regex to parse through the HTML. A lot of adjustment was needed to eliminate unintended Nones from
    # special cases...
    def parse_move(html):
        pattern = re.compile(
            r'<td class="cell-name"><a .* title="View details for (?P<move>[^"]+)">.*</a></td>'
            r' ?<td class="cell-icon"><a .*>(?P<type>[^<]+)</a></td>'
            r' ?<td class="cell-icon text-center".*data-sort-value="(?P<kind>[^"]*)".*</td>'
            r' ?<td class="cell-num">(?P<power>\d+|-)</td>'
            r' ?<td class="cell-num[^>]*>(?P<accuracy>\d+|-|&infin;)</td>'
            r' ?<td class="cell-num" ?>(?P<pp>\d+|-)</td>'
            r' ?<td class="cell-long-text" ?>(?P<desc>[^<]*)</td>'
            r' ?<td class="cell-num" ?>(?P<prob>\d+|-)</td>'
        )
        match = pattern.search(html)
        if not match:
            return None
        data = match.groupdict()
        for key in ['power', 'accuracy', 'pp', 'prob']:
            if data[key] == '-':
                data[key] = None
            elif data[key] == '&infin;':
                data[key] = "Infinity"
            else:
                data[key] = int(data[key])
        return data
    moves = []
    with open(os.path.join(DIR, 'moves', 'all_moves_through_gen_9.txt')) as file:
        next_line = file.readline().strip()
        while next_line:
            moves.append(parse_move(next_line+file.readline().strip()+file.readline().strip()))
            next_line = file.readline().strip()
        with open(os.path.join(DIR, 'moves', 'all_moves_through_gen_9.json'), 'w') as jf:
            json.dump(moves, jf)

def read_trainer_battles_data():
    DIR = os.path.join(START_DIR, 'data/run_and_bun')
    # I had to do some cleaning to the raw info from Trainer_Battles.txt found in Run and Bun documentation.
    # This is useful since it gives me the actual universe of possible enemy trainers, telling me Dyna/Gigamax
    # isn't in the game and Z-moves aren't either. It also gave me indications (by comparing set of moves to PokemonDB)
    # as to moves I had to make a consistent name for, and reminded me about the special case of Hidden Power (I'll
    # need to swap existing move data for the different-typed versions of it).
    def parse_moveset(moveset):
        pattern = re.compile(
            r'(?P<species>[A-Za-z\d_]+)\s+Lv\.(?P<lvl>\d+)\s+'
            r'(@(?P<item>[A-Za-z\s]+):\s*)?'
            r'(?P<moves>[^\[]*)\s*'
            r'\[(?P<nature>[^|]+)\|(?P<ability>[^]]+)\]'
            )
        # print(pattern.search("""Kirlia Lv.43 [Modest|Synchronize]""").groupdict())
        match = pattern.search(moveset)
        if not match:
            return None
        data = match.groupdict()
        data['moves'] = data['moves'].strip()
        data['moves'] = data['moves'].split(', ') if data['moves'] else []
        return data
    trainers = []
    with open(os.path.join(DIR, 'trainer_battles.txt')) as file:
        next_line = file.readline().strip()
        while next_line:
            new_trainer = {'name': next_line, 'team': []}
            next_line = file.readline().strip()
            while next_line:
                new_trainer['team'].append(parse_moveset(next_line))
                next_line = file.readline().strip()
            trainers.append(new_trainer)
            next_line = file.readline().strip()
        with open(os.path.join(DIR, 'trainer_battles.json'), 'w') as jf:
            json.dump(trainers, jf)

def check_move_universe():
    # Sanity-check set of all moves found in trainer battles and set of all moves through gen 8
    moves = set()
    trainer_moves = set()
    DIR = os.path.join(START_DIR, 'data', 'base_gen_8')
    with open(os.path.join(DIR, 'moves', 'all_moves_through_gen_9.json')) as file:
        m = json.load(file)
        for move in m:
            moves.add(move['move'].strip())
    with open(os.path.join(DIR, 'moves', 'all_moves_gen_9.json')) as file:
        m = json.load(file)
        for move in m:
            moves.remove(move.strip())
    DIR = os.path.join(START_DIR, 'data/run_and_bun')
    with open(os.path.join(DIR, 'trainer_battles.json')) as file:
        t = json.load(file)
        for trainer in t:
            for pkmn in trainer['team']:
                for move in pkmn['moves']:
                    trainer_moves.add(move.strip())
    no_imp = moves - trainer_moves
    g_max_moves = []
    max_moves = []
    z_moves = [
        'Breakneck Blitz',
        'All-Out Pummeling',
        'Supersonic Skystrike',
        'Acid Downpour',
        'Tectonic Rage',
        'Continental Crush',
        'Savage Spin-Out',
        'Never-Ending Nightmare',
        'Corkscrew Crash',
        'Inferno Overdrive',
        'Hydro Vortex',
        'Bloom Doom',
        'Gigavolt Havoc',
        'Shattered Psyche',
        'Subzero Slammer',
        'Devastating Drake',
        'Black Hole Eclipse',
        'Twinkle Tackle',
        'Catastropika',
        '10,000,000 Volt Thunderbolt',
        'Stoked Sparksurfer',
        'Extreme Evoboost',
        'Pulverizing Pancake',
        'Genesis Supernova',
        'Sinister Arrow Raid',
        'Malicious Moonsault',
        'Oceanic Operetta',
        'Splintered Stormshards',
        "Let's Snuggle Forever",
        'Clangorous Soulblaze',
        'Guardian of Alola',
        'Searing Sunraze Smash',
        'Menacing Moonraze Maelstrom',
        'Light That Burns the Sky',
        'Soul-Stealing 7-Star Strike',
    ]
    normal_moves = []
    for m in no_imp:
        if 'G-Max' in m:
            g_max_moves.append(m)
        elif 'Max' in m:
            max_moves.append(m)
        elif m not in z_moves:
            normal_moves.append(m)
    print(len(g_max_moves)) # 33
    print(len(max_moves)) # 19
    print(len(z_moves)) # 35
    print(len(normal_moves)) # 331
    print(len(trainer_moves)) # 475

    # print(normal_moves)
    # print()
    # print(trainer_moves - moves)
check_move_universe()