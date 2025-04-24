#include "ref.hpp"

#include <algorithm>
#include <format>
#include <optional>
#include <unordered_set>
#include <fstream>
#include <iostream>
#include <sstream>
namespace pkmn {
// GLOBAL VARIABLE
std::string DATA_FOLDER = "/Users/dd2/Documents/classes/4th_year/CPSC490/data/base_gen_8";
// std::string DATA_FOLDER = "/Users/dd2/Documents/classes/4th_year/CPSC490/data/run_and_bun";

// TYPE CHART
TypeChart type_chart;
TypeChart::TypeChart() {
  const std::unordered_set<std::pair<Type, Type>> super_eff = {
      {Type::FIRE, Type::GRASS},      {Type::FIRE, Type::ICE},
      {Type::FIRE, Type::BUG},        {Type::FIRE, Type::STEEL},
      {Type::WATER, Type::FIRE},      {Type::WATER, Type::GROUND},
      {Type::WATER, Type::ROCK},      {Type::ELECTRIC, Type::WATER},
      {Type::ELECTRIC, Type::FLYING}, {Type::GRASS, Type::WATER},
      {Type::GRASS, Type::GROUND},    {Type::GRASS, Type::ROCK},
      {Type::ICE, Type::GRASS},       {Type::ICE, Type::GROUND},
      {Type::ICE, Type::FLYING},      {Type::ICE, Type::DRAGON},
      {Type::FIGHTING, Type::NORMAL}, {Type::FIGHTING, Type::ICE},
      {Type::FIGHTING, Type::ROCK},   {Type::FIGHTING, Type::DARK},
      {Type::FIGHTING, Type::STEEL},  {Type::POISON, Type::GRASS},
      {Type::POISON, Type::FAIRY},    {Type::GROUND, Type::FIRE},
      {Type::GROUND, Type::ELECTRIC}, {Type::GROUND, Type::POISON},
      {Type::GROUND, Type::ROCK},     {Type::GROUND, Type::STEEL},
      {Type::FLYING, Type::GRASS},    {Type::FLYING, Type::FIGHTING},
      {Type::FLYING, Type::BUG},      {Type::PSYCHIC, Type::FIGHTING},
      {Type::PSYCHIC, Type::POISON},  {Type::BUG, Type::GRASS},
      {Type::BUG, Type::PSYCHIC},     {Type::BUG, Type::DARK},
      {Type::ROCK, Type::FIRE},       {Type::ROCK, Type::ICE},
      {Type::ROCK, Type::FLYING},     {Type::ROCK, Type::BUG},
      {Type::GHOST, Type::PSYCHIC},   {Type::GHOST, Type::GHOST},
      {Type::DRAGON, Type::DRAGON},   {Type::DARK, Type::PSYCHIC},
      {Type::DARK, Type::GHOST},      {Type::STEEL, Type::ICE},
      {Type::STEEL, Type::ROCK},      {Type::STEEL, Type::FAIRY},
      {Type::FAIRY, Type::FIGHTING},  {Type::FAIRY, Type::DRAGON},
      {Type::FAIRY, Type::DARK}};
  const std::unordered_set<std::pair<Type, Type>> not_eff = {
      {Type::NORMAL, Type::ROCK},     {Type::NORMAL, Type::STEEL},
      {Type::FIRE, Type::FIRE},       {Type::FIRE, Type::WATER},
      {Type::FIRE, Type::ROCK},       {Type::FIRE, Type::DRAGON},
      {Type::WATER, Type::WATER},     {Type::WATER, Type::GRASS},
      {Type::WATER, Type::DRAGON},    {Type::ELECTRIC, Type::ELECTRIC},
      {Type::ELECTRIC, Type::GRASS},  {Type::ELECTRIC, Type::DRAGON},
      {Type::GRASS, Type::FIRE},      {Type::GRASS, Type::GRASS},
      {Type::GRASS, Type::POISON},    {Type::GRASS, Type::FLYING},
      {Type::GRASS, Type::BUG},       {Type::GRASS, Type::DRAGON},
      {Type::GRASS, Type::STEEL},     {Type::ICE, Type::FIRE},
      {Type::ICE, Type::WATER},       {Type::ICE, Type::ICE},
      {Type::ICE, Type::STEEL},       {Type::FIGHTING, Type::POISON},
      {Type::FIGHTING, Type::FLYING}, {Type::FIGHTING, Type::PSYCHIC},
      {Type::FIGHTING, Type::BUG},    {Type::FIGHTING, Type::FAIRY},
      {Type::POISON, Type::POISON},   {Type::POISON, Type::GROUND},
      {Type::POISON, Type::ROCK},     {Type::POISON, Type::GHOST},
      {Type::GROUND, Type::GRASS},    {Type::GROUND, Type::BUG},
      {Type::FLYING, Type::ELECTRIC}, {Type::FLYING, Type::ROCK},
      {Type::FLYING, Type::STEEL},    {Type::PSYCHIC, Type::PSYCHIC},
      {Type::PSYCHIC, Type::STEEL},   {Type::BUG, Type::FIRE},
      {Type::BUG, Type::FIGHTING},    {Type::BUG, Type::POISON},
      {Type::BUG, Type::FLYING},      {Type::BUG, Type::GHOST},
      {Type::BUG, Type::STEEL},       {Type::BUG, Type::FAIRY},
      {Type::ROCK, Type::FIGHTING},   {Type::ROCK, Type::GROUND},
      {Type::ROCK, Type::STEEL},      {Type::GHOST, Type::DARK},
      {Type::DRAGON, Type::STEEL},    {Type::DARK, Type::FIGHTING},
      {Type::DARK, Type::DARK},       {Type::DARK, Type::FAIRY},
      {Type::STEEL, Type::FIRE},      {Type::STEEL, Type::WATER},
      {Type::STEEL, Type::ELECTRIC},  {Type::STEEL, Type::STEEL},
      {Type::FAIRY, Type::FIRE},      {Type::FAIRY, Type::POISON},
      {Type::FAIRY, Type::STEEL}};
  const std::unordered_set<std::pair<Type, Type>> immune = {
      {Type::NORMAL, Type::GHOST}, {Type::ELECTRIC, Type::GROUND}, {Type::FIGHTING, Type::GHOST},
      {Type::POISON, Type::STEEL}, {Type::GROUND, Type::FLYING},   {Type::PSYCHIC, Type::DARK},
      {Type::GHOST, Type::NORMAL}, {Type::DRAGON, Type::FAIRY}};
  for (int i = 1; i < (int)Type::NO_TYPE; i++) { // We know that there are 18 types; 0 is NONE
    Type attacker = (Type)i;
    for (int j = 1; j < (int)Type::NO_TYPE; j++) {
      Type defender = (Type)j;
      std::pair<Type, Type> type_pair = {attacker, defender};
      if (super_eff.find(type_pair) != super_eff.end()) {
        chart[attacker][defender] = 1; // Super effective
      } else if (not_eff.find(type_pair) != not_eff.end()) {
        chart[attacker][defender] = -1; // Not very effective
      } else if (immune.find(type_pair) != immune.end()) {
        chart[attacker][defender] = -2; // Immune
      } else {
        chart[attacker][defender] = 0; // Neutral
      }
    }
  }
}
int getTypeEffectiveness(Type attacker, Type defender) {
  return type_chart.chart[attacker][defender];
}

// BASE STATS and TYPES
PokeDexDict pokedex;
PokeDexDict::PokeDexDict() {
  std::ifstream file(std::format("{}/species_data.txt", DATA_FOLDER));
  if (!file) {
    std::cerr << "Error opening Pokedex datafile!" << std::endl;
  }
  int i = 0;
  std::string line;
  while (std::getline(file, line)) {
    if (++i >= (int)PokeName::END) {
      std::cerr << "Extra line while reading stats' datafile:" << std::endl;
      std::cerr << line << std::endl;
      break;
    }
    std::istringstream iss(line);
    int species;
    Stats stats{};
    int a, b;
    iss >> species >> stats.hp >> stats.att >> stats.def >> stats.spatt >> stats.spdef >>
        stats.spd >> a >> b;
    PokeName name = (PokeName)i;
    base_stat_dict[name] = stats;
    type_dict[name] = std::pair<Type, Type>{static_cast<Type>(a), static_cast<Type>(b)};
    species_dict[name] = (Species)species;
  }
  file.close();
}

// MOVES
MoveDict moveDict;
std::optional<int> get_opt_nonneg_int(int val) {
  return val >= 0 ? val : std::optional<int>(std::nullopt);
}
MoveDict::MoveDict() {
  int NUM_FIELDS = 11;
  // Read from file
  std::ifstream file(std::format("{}/move_data/basic_move_data.txt", DATA_FOLDER));
  if (!file) {
    std::cerr << "Error opening moves' datafile!" << std::endl;
  }
  std::string line;
  int tmp[NUM_FIELDS];
  std::unordered_set<MoveId> soundMoves = {
      MoveId::ALLURINGVOICE,  MoveId::BOOMBURST,      MoveId::BUGBUZZ,      MoveId::CHATTER,
      MoveId::CLANGINGSCALES, MoveId::CLANGOROUSSOUL, MoveId::CONFIDE,      MoveId::DISARMINGVOICE,
      MoveId::ECHOEDVOICE,    MoveId::EERIESPELL,     MoveId::GRASSWHISTLE, MoveId::GROWL,
      MoveId::HEALBELL,       MoveId::HOWL,           MoveId::HYPERVOICE,   MoveId::METALSOUND,
      MoveId::NOBLEROAR,      MoveId::OVERDRIVE,      MoveId::PARTINGSHOT,  MoveId::PERISHSONG,
      MoveId::PSYCHICNOISE,   MoveId::RELICSONG,      MoveId::ROAR,         MoveId::ROUND,
      MoveId::SCREECH,        MoveId::SING_,          MoveId::SNARL,        MoveId::SNORE,
      MoveId::SPARKLINGARIA,  MoveId::SUPERSONIC,     MoveId::TORCHSONG,    MoveId::UPROAR};
  for (int j = 0; j <= (int)MoveId::END; j++) {
    MoveId id = (MoveId)j;
    if (!std::getline(file, line)) {
      std::cerr << "Not enough lines while reading moves' datafile:" << std::endl;
      break;
    }
    std::istringstream iss(line);
    Move ret{};
    for (int k = 0; k < NUM_FIELDS; k++) {
      iss >> tmp[k];
    }
    int ind = 0;
    ret.id = id;
    ret.type = static_cast<Type>(tmp[ind++]);             // 0
    ret.basePower = tmp[ind++];                           // 1
    ret.accuracy = get_opt_nonneg_int(tmp[ind++]);        // 2
    ret.pp = tmp[ind++];                                  // 3
    ret.critRatio = tmp[ind++];                           // 4
    ret.priority = tmp[ind++];                            // 5
    ret.category = static_cast<MoveCategory>(tmp[ind++]); // 6
    ret.status = static_cast<Status>(tmp[ind++]);         // 7
    ret.weather = static_cast<Weather>(tmp[ind++]);       // 8
    ret.terrain = static_cast<Terrain>(tmp[ind++]);       // 9
    ret.target = static_cast<Target>(tmp[ind++]);         // 10
    if(std::find(soundMoves.begin(), soundMoves.end(), id) != soundMoves.end()) {
      ret.sound = true;
    }
    dict[id] = ret;
  }
  if (std::getline(file, line)) {
    std::cerr << "Extra line(s) while reading moves' datafile:" << std::endl;
    std::cerr << line << std::endl;
  }
  file.close();
  patch_move_dict();
}
// Apply Run and Bun mechanic changes
void MoveDict::patch_move_dict() {
  // Absorb BP 20 -> 40
  moveDict.dict[MoveId::ABSORB].basePower = 40;
  // Aeroblast acc 95 -> 100
  moveDict.dict[MoveId::AEROBLAST].accuracy = {100};
  // AirCutter acc 95 -> 100
  moveDict.dict[MoveId::AIRCUTTER].accuracy = {100};
  // AirSlash acc 95 -> 100
  moveDict.dict[MoveId::AIRSLASH].accuracy = {100};
  // AquaTail acc 90 -> 95
  moveDict.dict[MoveId::AQUATAIL].accuracy = {95};
  // Astonish BP 30 -> 40
  moveDict.dict[MoveId::ASTONISH].basePower = 40;
  // BabyDollEyes PP 30 -> 10
  moveDict.dict[MoveId::BABYDOLLEYES].pp = 10;
  // Barrage acc 85 -> 100
  moveDict.dict[MoveId::BARRAGE].accuracy = {100};
  // Belch acc 90 -> 100
  moveDict.dict[MoveId::BELCH].accuracy = {100};
  // Bind acc 85 -> 100
  moveDict.dict[MoveId::BIND].accuracy = {100};
  // BlazeKick acc 90 -> 100
  moveDict.dict[MoveId::BLAZEKICK].accuracy = {100};
  // Blizzard acc 70 -> 80
  moveDict.dict[MoveId::BLIZZARD].accuracy = {80};
  // BlueFlare acc 85 -> 90
  moveDict.dict[MoveId::BLUEFLARE].accuracy = {90};
  // BoltStrike acc 85 -> 90
  moveDict.dict[MoveId::BOLTSTRIKE].accuracy = {90};
  // BoneClub acc 85 -> 100
  moveDict.dict[MoveId::BONECLUB].accuracy = {100};
  // Bonemerang acc 90 -> 100
  moveDict.dict[MoveId::BONEMERANG].accuracy = {100};
  // Bounce acc 85 -> 95
  moveDict.dict[MoveId::BOUNCE].accuracy = {95};
  // Captivate PP 20 -> 5
  moveDict.dict[MoveId::CAPTIVATE].pp = 5;
  // ChargeBeam BP 50 -> 40
  moveDict.dict[MoveId::CHARGEBEAM].basePower = 40;
  // ChargeBeam acc 90 -> 100
  moveDict.dict[MoveId::CHARGEBEAM].accuracy = {100};
  // Charm PP 20 -> 5
  moveDict.dict[MoveId::CHARM].pp = 5;
  // CircleThrow acc 90 -> 95
  moveDict.dict[MoveId::CIRCLETHROW].accuracy = {95};
  // Clamp acc 85 -> 100
  moveDict.dict[MoveId::CLAMP].accuracy = {100};
  // CometPunch acc 85 -> 90
  moveDict.dict[MoveId::COMETPUNCH].accuracy = {100};
  // Confide PP 20 -> 10
  moveDict.dict[MoveId::CONFIDE].pp = 10;
  // Covet Type Normal -> Fairy
  moveDict.dict[MoveId::COVET].type = Type::FAIRY;
  // Crabhammer acc 90 -> 100
  moveDict.dict[MoveId::CRABHAMMER].accuracy = {100};
  // Cut acc 95 -> 100
  moveDict.dict[MoveId::CUT].accuracy = {100};
  // DarkVoid acc 50 -> 80
  moveDict.dict[MoveId::DARKVOID].accuracy = {80};
  // DiamondStorm acc 90 -> 100
  moveDict.dict[MoveId::DIAMONDSTORM].accuracy = {100};
  // DoubleHit acc 90 -> 100
  moveDict.dict[MoveId::DOUBLEHIT].accuracy = {100};
  // DoubleSlap acc 85 -> 100
  moveDict.dict[MoveId::DOUBLESLAP].accuracy = {100};
  // DracoMeteor acc 90 -> 100
  moveDict.dict[MoveId::DRACOMETEOR].accuracy = {100};
  // DragonRush acc 75 -> 85
  moveDict.dict[MoveId::DRAGONRUSH].accuracy = {85};
  // DragonTail acc 90 -> 95
  moveDict.dict[MoveId::DRAGONTAIL].accuracy = {95};
  // DrillRun acc 95 -> 100
  moveDict.dict[MoveId::DRILLRUN].accuracy = {100};
  // DualChop acc 90 -> 100
  moveDict.dict[MoveId::DUALCHOP].accuracy = {100};
  // DualWingbeat acc 90 -> 100
  moveDict.dict[MoveId::DUALWINGBEAT].accuracy = {100};
  // EerieImpulse PP 15 -> 5
  moveDict.dict[MoveId::EERIEIMPULSE].pp = 5;
  // Electroweb acc 95 -> 100
  moveDict.dict[MoveId::ELECTROWEB].accuracy = {100};
  // FakeOut PP 10 -> 5
  moveDict.dict[MoveId::FAKEOUT].pp = 5;
  // FakeTears PP 20 -> 5
  moveDict.dict[MoveId::FAKETEARS].pp = 5;
  // FeatherDance PP 15 -> 5
  moveDict.dict[MoveId::FEATHERDANCE].pp = 5;
  // FireFang acc 95 -> 100
  moveDict.dict[MoveId::FIREFANG].accuracy = {100};
  // FireSpin acc 85 -> 100
  moveDict.dict[MoveId::FIRESPIN].accuracy = {100};
  // Flash acc 100 -> 70
  moveDict.dict[MoveId::FLASH].accuracy = {70};
  // FleurCannon acc 90 -> 100
  moveDict.dict[MoveId::FLEURCANNON].accuracy = {100};
  // Fly acc 95 -> 100
  moveDict.dict[MoveId::FLY].accuracy = {100};
  // FlyingPress acc 95 -> 100
  moveDict.dict[MoveId::FLYINGPRESS].accuracy = {100};
  // FocusBlast acc 70 -> 80
  moveDict.dict[MoveId::FOCUSBLAST].accuracy = {80};
  // FreezeShock acc 90 -> 100
  moveDict.dict[MoveId::FREEZESHOCK].accuracy = {100};
  // FrenzyPlant acc 90 -> 100
  moveDict.dict[MoveId::FRENZYPLANT].accuracy = {100};
  // FrostBreath acc 90 -> 100
  moveDict.dict[MoveId::FROSTBREATH].accuracy = {100};
  // Frustraion BP ?? -> 102
  moveDict.dict[MoveId::FRUSTRATION].basePower = 102;
  // FuryAttack acc 85 -> 100
  moveDict.dict[MoveId::FURYATTACK].accuracy = {100};
  // FurySwipes acc 80 -> 90
  moveDict.dict[MoveId::FURYSWIPES].accuracy = {90};
  // GearGrind acc 85 -> 100
  moveDict.dict[MoveId::GEARGRIND].accuracy = {100};
  // GigaImpact acc 90 -> 100
  moveDict.dict[MoveId::GIGAIMPACT].accuracy = {100};
  // Glaciate acc 95 -> 100
  moveDict.dict[MoveId::GLACIATE].accuracy = {100};
  // GrassWhistle acc 55 -> 70
  moveDict.dict[MoveId::GRASSWHISTLE].accuracy = {70};
  // Growl PP 40 -> 10
  moveDict.dict[MoveId::GROWL].pp = 10;
  // GunkShot acc 80 -> 85
  moveDict.dict[MoveId::GUNKSHOT].accuracy = {85};
  // HammerArm acc 90 -> 100
  moveDict.dict[MoveId::HAMMERARM].accuracy = {100};
  // Harden PP 40 -> 5
  moveDict.dict[MoveId::HARDEN].pp = 5;
  // HeadSmash acc 80 -> 85
  moveDict.dict[MoveId::HEADSMASH].accuracy = {85};
  // HeatWave acc 90 -> 100
  moveDict.dict[MoveId::HEATWAVE].accuracy = {100};
  // HighHorsepower acc 95 -> 100
  moveDict.dict[MoveId::HIGHHORSEPOWER].accuracy = {100};
  // Hurricane acc 70 -> 80
  moveDict.dict[MoveId::HURRICANE].accuracy = {80};
  // HydroCannon acc 90 -> 100
  moveDict.dict[MoveId::HYDROCANNON].accuracy = {100};
  // HydroPump acc 80 -> 85
  moveDict.dict[MoveId::HYDROPUMP].accuracy = {85};
  // HyperBeam acc 90 -> 100
  moveDict.dict[MoveId::HYPERBEAM].accuracy = {100};
  // HyperFang acc 90 -> 100
  moveDict.dict[MoveId::HYPERFANG].accuracy = {100};
  // Hypnosis acc 60 -> 70
  moveDict.dict[MoveId::HYPNOSIS].accuracy = {70};
  // IceBurn acc 90 -> 100
  moveDict.dict[MoveId::ICEBURN].accuracy = {100};
  // IceFang acc 95 -> 100
  moveDict.dict[MoveId::ICEFANG].accuracy = {100};
  // IceHammer acc 90 -> 100
  moveDict.dict[MoveId::ICEHAMMER].accuracy = {100};
  // IcicleCrash acc 95 -> 100
  moveDict.dict[MoveId::ICICLECRASH].accuracy = {100};
  // IcyWind acc 95 -> 100
  moveDict.dict[MoveId::ICYWIND].accuracy = {100};
  // IronTail acc 75 -> 85
  moveDict.dict[MoveId::IRONTAIL].accuracy = {85};
  // Kinesis acc 80 -> 100
  moveDict.dict[MoveId::KINESIS].accuracy = {100};
  // LeafStorm acc 90 -> 100
  moveDict.dict[MoveId::LEAFSTORM].accuracy = {100};
  // LeafTornado acc 90 -> 100
  moveDict.dict[MoveId::LEAFTORNADO].accuracy = {100};
  // LeechSeed acc 90 -> 100
  moveDict.dict[MoveId::LEECHSEED].accuracy = {100};
  // Leer PP 30 -> 10
  moveDict.dict[MoveId::LEER].pp = 10;
  // Lick BP 30 -> 40
  moveDict.dict[MoveId::LICK].basePower = 40;
  // LightOfRuin acc 90 -> 100
  moveDict.dict[MoveId::LIGHTOFRUIN].accuracy = {100};
  // LovelyKiss acc 75 -> 80
  moveDict.dict[MoveId::LOVELYKISS].accuracy = {80};
  // MagmaStorm acc 75 -> 90
  moveDict.dict[MoveId::MAGMASTORM].accuracy = {90};
  // MegaDrain BP 40 -> 60
  moveDict.dict[MoveId::MEGADRAIN].basePower = 60;
  // MegaKick acc 75 -> 85
  moveDict.dict[MoveId::MEGAKICK].accuracy = {85};
  // MegaPunch acc 85 -> 100
  moveDict.dict[MoveId::MEGAPUNCH].accuracy = {100};
  // MegaHorn acc 85 -> 90
  moveDict.dict[MoveId::MEGAHORN].accuracy = {90};
  // MetalClaw acc 95 -> 100
  moveDict.dict[MoveId::METALCLAW].accuracy = {100};
  // MetalSound acc 85 -> 100
  moveDict.dict[MoveId::METALSOUND].accuracy = {100};
  // MetalSound PP 40 -> 5
  moveDict.dict[MoveId::METALSOUND].pp = 5;
  // MeteorBeam acc 90 -> 100
  moveDict.dict[MoveId::METEORBEAM].accuracy = {100};
  // MeteorMash acc 90 -> 100
  moveDict.dict[MoveId::METEORMASH].accuracy = {100};
  // MirrorShot acc 85 -> 100
  moveDict.dict[MoveId::MIRRORSHOT].accuracy = {100};
  // MistyExplosion BP 100 -> 200
  moveDict.dict[MoveId::MISTYEXPLOSION].basePower = 200;
  // MudBomb acc 85 -> 100
  moveDict.dict[MoveId::MUDBOMB].accuracy = {100};
  // MuddyWater acc 85 -> 95
  moveDict.dict[MoveId::MUDDYWATER].accuracy = {95};
  // NaturesMadness acc 90 -> 100
  moveDict.dict[MoveId::NATURESMADNESS].accuracy = {100};
  // NightDaze acc 95 -> 100
  moveDict.dict[MoveId::NIGHTDAZE].accuracy = {100};
  // NobleRoar PP 30 -> 10
  moveDict.dict[MoveId::NOBLEROAR].pp = {10};
  // Octazooka BP 65 -> 80
  moveDict.dict[MoveId::OCTAZOOKA].basePower = 80;
  // Octazooka acc 85 -> 100
  moveDict.dict[MoveId::OCTAZOOKA].accuracy = {100};
  // OriginPulse acc 85 -> 100
  moveDict.dict[MoveId::ORIGINPULSE].accuracy = {100};
  // Overheat acc 90 -> 100
  moveDict.dict[MoveId::OVERHEAT].accuracy = {100};
  // PinMissile acc 95 -> 100
  moveDict.dict[MoveId::PINMISSILE].accuracy = {100};
  // PlayNice PP 20 -> 10
  moveDict.dict[MoveId::PLAYNICE].pp = 10;
  // PlayRough acc 90 -> 100
  moveDict.dict[MoveId::PLAYROUGH].accuracy = {100};
  // PoisonPowder acc 75 -> 90
  moveDict.dict[MoveId::POISONPOWDER].accuracy = {90};
  // PowerWhip acc 85 -> 90
  moveDict.dict[MoveId::POWERWHIP].accuracy = {90};
  // PrecipiceBlades acc 85 -> 100
  moveDict.dict[MoveId::PRECIPICEBLADES].accuracy = {100};
  // PsychoBoost acc 90 -> 100
  moveDict.dict[MoveId::PSYCHOBOOST].accuracy = {100};
  // RazorLeaf acc 95 -> 100
  moveDict.dict[MoveId::RAZORLEAF].accuracy = {100};
  // RazorShell acc 95 -> 100
  moveDict.dict[MoveId::RAZORSHELL].accuracy = {100};
  // Return BP ?? -> 102
  moveDict.dict[MoveId::RETURN].basePower = 102;
  // RoarOfTime acc 90 -> 100
  moveDict.dict[MoveId::ROAROFTIME].accuracy = {100};
  // RockBlast acc 90 -> 100
  moveDict.dict[MoveId::ROCKBLAST].accuracy = {100};
  // RockClimb acc 85 -> 95
  moveDict.dict[MoveId::ROCKCLIMB].accuracy = {95};
  // RockSlide acc 90 -> 100
  moveDict.dict[MoveId::ROCKSLIDE].accuracy = {100};
  // RockThrow acc 90 -> 100
  moveDict.dict[MoveId::ROCKTHROW].accuracy = {100};
  // RockWrecker acc 90 -> 100
  moveDict.dict[MoveId::ROCKWRECKER].accuracy = {100};
  // RollingKick acc 85 -> 100
  moveDict.dict[MoveId::ROLLINGKICK].accuracy = {100};
  // Roost PP 10 -> 5
  moveDict.dict[MoveId::ROOST].pp = 5;
  // SacredFire acc 95 -> 100
  moveDict.dict[MoveId::SACREDFIRE].accuracy = {100};
  // SandTomb acc 85 -> 100
  moveDict.dict[MoveId::SANDTOMB].accuracy = {100};
  // SandAttack PP 15 -> 5
  moveDict.dict[MoveId::SANDATTACK].pp = 5;
  // ScaleShot acc 90 -> 100
  moveDict.dict[MoveId::SCALESHOT].accuracy = {100};
  // Screech acc 85 -> 100
  moveDict.dict[MoveId::SCREECH].accuracy = {100};
  // Screech PP 15 -> 5
  moveDict.dict[MoveId::SCREECH].pp = 5;
  // SeedFlare acc 85 -> 90
  moveDict.dict[MoveId::SEEDFLARE].accuracy = {90};
  // Sing acc 55 -> 70
  moveDict.dict[MoveId::SING_].accuracy = {70};
  // SkitterSmack acc 90 -> 100
  moveDict.dict[MoveId::SKITTERSMACK].accuracy = {100};
  // SkyAttack acc 90 -> 100
  moveDict.dict[MoveId::SKYATTACK].accuracy = {100};
  // SkyUppercut acc 90 -> 100
  moveDict.dict[MoveId::SKYUPPERCUT].accuracy = {100};
  // Slam acc 75 -> 90
  moveDict.dict[MoveId::SLAM].accuracy = {90};
  // SleepPowder acc 75 -> 80
  moveDict.dict[MoveId::SLEEPPOWDER].accuracy = {80};
  // Smog acc 70 -> 90
  moveDict.dict[MoveId::SMOG].accuracy = {90};
  // Snarl acc 95 -> 100
  moveDict.dict[MoveId::SNARL].accuracy = {100};
  // Snarl PP 15 -> 10
  moveDict.dict[MoveId::SNARL].pp = 10;
  // SonicBoomb acc 90 -> 100
  moveDict.dict[MoveId::SONICBOOM].accuracy = {100};
  // SpacialRend acc 95 -> 100
  moveDict.dict[MoveId::SPACIALREND].accuracy = {100};
  // SteamEruption acc 95 -> 100
  moveDict.dict[MoveId::STEAMERUPTION].accuracy = {100};
  // SteelBeam acc 95 -> 100
  moveDict.dict[MoveId::STEELBEAM].accuracy = {100};
  // StoneEdge acc 80 -> 85
  moveDict.dict[MoveId::STONEEDGE].accuracy = {85};
  // StrangeSteam acc 95 -> 100
  moveDict.dict[MoveId::STRANGESTEAM].accuracy = {100};
  // StruggleBug PP 20 -> 10
  moveDict.dict[MoveId::STRUGGLEBUG].pp = 10;
  // StunSpore acc 75 -> 90
  moveDict.dict[MoveId::STUNSPORE].accuracy = {90};
  // Submission acc 80 -> 100
  moveDict.dict[MoveId::SUBMISSION].accuracy = {100};
  // SuperFang acc 90 -> 100
  moveDict.dict[MoveId::SUPERFANG].accuracy = {100};
  // SuperFang Type Normal -> Dark
  moveDict.dict[MoveId::SUPERFANG].type = Type::DARK;
  // Supersonic acc 55 -> 70
  moveDict.dict[MoveId::SUPERSONIC].accuracy = {70};
  // Swagger acc 85 -> 90
  moveDict.dict[MoveId::SWAGGER].accuracy = {90};
  // SweetKiss acc 75 -> 80
  moveDict.dict[MoveId::SWEETKISS].accuracy = {80};
  // TailSlap acc 85 -> 100
  moveDict.dict[MoveId::TAILSLAP].accuracy = {100};
  // Takedown acc 85 -> 100
  moveDict.dict[MoveId::TAKEDOWN].accuracy = {100};
  // TeafulLook PP 20 -> 10
  moveDict.dict[MoveId::TEARFULLOOK].pp = 10;
  // Thunder acc 70 -> 80
  moveDict.dict[MoveId::THUNDER].accuracy = {80};
  // ThunderCage acc 90 -> 100
  moveDict.dict[MoveId::THUNDERCAGE].accuracy = {100};
  // ThunderFang acc 95 -> 100
  moveDict.dict[MoveId::THUNDERFANG].accuracy = {100};
  // Tickle PP 20 -> 10
  moveDict.dict[MoveId::TICKLE].pp = 10;
  // Whirlpool acc 85 -> 100
  moveDict.dict[MoveId::WHIRLPOOL].accuracy = {100};
  // Wrap acc 90 -> 100
  moveDict.dict[MoveId::WRAP].accuracy = {100};
  // ZenHeadbutt acc 90 -> 100
  moveDict.dict[MoveId::ZENHEADBUTT].accuracy = {100};
}
} // namespace pkmn