#pragma once

#include <optional>
#include <random>
namespace pkmn::math {
// int odd_round(int a, int b, int c) {
//   // Return (a*b)/c, rounding to the nearest integer but rounding down at 0.5
//   double result = static_cast<double>(a * b) / c;
//   // Round to the nearest integer, but round down at exactly 0.5
//   int rounded = (result - std::floor(result) == 0.5) ? std::floor(result) : std::round(result);
//   return static_cast<int>(rounded);
// }

// Return a random 32-bit integer.
uint32_t randomUint32() {
  // Here I just use ChatGPT-provided RNG code template
  // TODO: Replace with faithful PRNG
  static std::random_device rd;  // Seed source (may be slow)
  static std::mt19937 gen(rd()); // Mersenne Twister PRNG
  static std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);
  return dist(gen);
}
// Return a random double in [0,1).
double random() { return static_cast<double>(randomUint32()) / static_cast<double>(1 >> 32); }
// Return a random int in [0,x). Requires x > 0.
int random(int x) { return static_cast<int>(static_cast<double>(x) * random()); }
// Return a random int in [x, y). Requires y > x.
int random(int x, int y) { return random(y - x) + x; }
// TODO: replace with faithful PRNG
bool randomChance(int numerator, int denominator) {
  // return false; // Pokemon Showdown has an option to force random chance
  return random(denominator) < numerator;
}
} // namespace pkmn::math