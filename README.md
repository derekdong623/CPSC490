# A Performant Game Engine and Monte-Carlo Tree Search Agent for Pokemon Battles
This repo has the source code for my Yale Computer Science undergraduate senior project. It contains much of the infrastructure needed to replicate the mechanics of [Pokemon Run and Bun](https://www.pokecommunity.com/threads/pok%C3%A9mon-run-bun-v1-07.493223/) battles written in a C++ library that can be imported to a Python program. Many specific mechanics are unimplemented due to time constraints, but I plan to continue working on adding and testing these features after graduation.
## How to use
Interested in trying out the engine for yourself? 
1. `cd ./src` and run `make pybind`. This should generate a shared object file: `src/build/bin/fast_pkmn.cpython-312-darwin.so`. See `src/Makefile` for information on how this is done.
2. If you'd like to use the `PokemonGame` class/interface, import `src/tests_python/pokemon_game.py` into your Python program. See `mcts.py` or `test_mcts.py` in `src/tests_python/` for examples.
3. If you'd like to directly access the compiled shared object file from a Python program, add to the beginning of your Python script:
```
import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "PATH_TO_BIN_FOLDER")))
```
Here, `PATH_TO_BIN_FOLDER` should be the relative path from the Python script's directory to the compiled shared object file. See `basic.py` or `pokemon_game.py` in `src/tests_python` for examples.

## Quick start directory guide
Confused by file structure? Start here for a quick overview of relevant directories.
- `data/`
  - You can safely ignore `run_and_bun/`.
  - `move_data/`
    - Raw numeric values read by engine (see `ref.cpp`). We make frequent use of enum values being represented in C++ by integers.
    - Ordering consistent with `enum MoveId` (see `list_move_ids.txt`). Note the move Sing has the ID SING_ since `SING` is a C keyword.
  - `species_data.txt`
    - Raw numeric values read by engine (see `ref.cpp`).
- `utils/`
  - Contains a single large Jupyter notebook used for parsing data files.
  - A lot was initially useful for parsing HTML tables scraped from e.g. [PokemonDB](https://pokemondb.net/), but eventually more complete datasets were exported directly from Pokemon Showdown.
  - Eventually became useful for e.g. summarizing possible self- or secondary effects of moves from the Showdown-exported files, or for summarizing experiment logs.
- `ps_trials/`
  - Can usually be ignored. `test_dex.js` gives rough examples of how to export Pokemon Showdown data into text files for further parsing.
- `src/`
  - `include/` and `src/`
    - Most of the meat for the engine itself.
    - The callbacks files contain implementations of e.g. `onTryHit()` and other event callbacks implemented and run in Pokemon Showdown. They are separated from other class members for clarity.
    - `sketch.txt` was useful for mapping control flow in Pokemon Showdown, but can now be ignored.
  - `tests/`
    - A quick and dirty framework for writing assertion-style tests for mechanics implemented in the engine. All tests can be run at once with `make test`.
    - Tests for mechanics are in `test_battle.cpp`.
  - `tests_python/`
    - Source code, data, and experiment logs for the basic MCTS agent implemented.
  - `bindings/bindings.cpp`
    - After compiling the object files, `make pybind` references this file to determine what (and how) to expose to Python in the shared object.
  - `build/`
    - Where the final object (and Python-interface shared object or test-running executable) files are built.
- `submitted_docs/`
  - Just contains documents submitted for the course throughout the year.