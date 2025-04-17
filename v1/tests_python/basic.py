import sys
import os

# Add the compiled module path to sys.path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "../build/bin")))

from fast_pkmn import *

print(PokeName.ZIGZAGOON)