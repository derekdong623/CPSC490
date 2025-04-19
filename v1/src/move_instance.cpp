#include "move_instance.hpp"
#include "utils.hpp"
#include <unordered_set>
namespace pkmn {
bool MoveInstance::onModifyType(Pokemon const &pokemon) {
  static const std::vector<MoveId> noModifyType = {
      MoveId::JUDGMENT,    MoveId::MULTIATTACK,  MoveId::NATURALGIFT, MoveId::REVELATIONDANCE,
      MoveId::TECHNOBLAST, MoveId::TERRAINPULSE, MoveId::WEATHERBALL};
  if (std::find(noModifyType.begin(), noModifyType.end(), id) == noModifyType.end()) {
    if (pokemon.ability == Ability::NORMALIZE) {
      moveData.type = Type::NORMAL;
    }
    if (moveData.type == Type::NORMAL) {
      switch (pokemon.ability) {
      case Ability::AERILATE:
        moveData.type = Type::FLYING;
      case Ability::GALVANIZE:
        moveData.type = Type::ELECTRIC;
      case Ability::PIXILATE:
        moveData.type = Type::FAIRY;
      case Ability::REFRIGERATE:
        moveData.type = Type::ICE;
      default:
        break;
      }
    }
    // typeChangerBoosted = this.effect;
  }
  if (pokemon.ability == Ability::LIQUID_VOICE && moveData.sound)
    moveData.type = Type::WATER;
  // TODO: Judgement, MultiAttack, TechnoBlast
  // TerrainPulse
  // WeatherBall
  // RagingBull
  // RevelationDance
  // NaturalGift
  // TODO: AuraWheel, HiddenPower
  // TODO: Electrify, IonDeluge
  return true;
}
bool MoveInstance::onModifyMove(Pokemon const &pokemon) {
  // PRIORITY 1
  // PropellerTail and Stalwart: do at getTarget level
  // StanceChange: formeChange
  // PRIORITY 0
  // TODO: GorillaTactics and Choice items...why GorillaTactics implemented a bit different?
  // Illuminate: N/A, only Gen 9
  if (pokemon.has_ability(Ability::INFILTRATOR)) {
    infiltrates = true;
  }
  // move.infiltrates = true; // Infiltrator
  // move.ignoreEvasion = true; // KeenEye
  // move.contact = false; // LongReach, PunchingGlove+punch
  // move.ignoreAbility = true; // MoldBreaker, MyceliumMight+Status, Teravolt, Turboblaze,
  // move.protect = false; // UnseenFist
  // if(move.moveData.defrost) { // Defrosting
  //   pokemon.clearStatus;
  // }
  // SkillLink: max roll or, for multiaccuracy, one accuracy check
  // LoadedDice: delete multiaccuracy only?
  // BeatUp: very complicated
  // BleakWindstorm, SandsearStorm, WildboltStorm: rain/primordial -> always hits
  // Blizzard: hail -> always hits
  // Growth: sun/desolate -> double boosts
  // Hurricane, Thunder: rain/primordial -> always, sun/desolate -> 50 acc
  // Curse: check Gen8 mod
  // ExpandingForce: N/A in singles
  // Pledge: N/A in singles
  // Gravity: Might cancel move by returning false
  // HealBlock: complicated
  // IceBall/Rollout: complicated
  // LightThatBurnsTheSky: N/A in Run&Bun
  // Magnitude: roll for power
  // PhotonGeyser: swap to physical if higher att
  // Present: skip for now
  // Pursuit: always hits if target being called back or switch flag
  // SecretPower: complicated
  // ShellsideArm: complicated
  // SkyDrop: complicated
  // Struggle: give NoneType
  // TeraBlast, TeraStarstorm: N/A, only Gen 9
  // if(field.terrain && pokemon.isGrounded()) {basePower *= 2;} // TerrainPulse
  // if(field.weather != Weather::NONE) {basePower *= 2;} // WeatherBall
  // ThroatChop
  // WonderRoom: extremely complicated
  // PRIORITY -1
  // Stench, KingsRock, RazorFang: push flinch secondary
  // PRIORITY -2
  // SereneGrace: double chance of secondary
  // SheerForce: delete secondaries and self-damage etc.
  // PRIORITY -5
  // Mind's Eye: N/A, only Gen 9 Bloodmoon Ursaluna
  // Scrappy
  return true;
}
//
bool MoveInstance::breaksProtect() {
  static const std::vector<MoveId> listOfMoves = {
      MoveId::FEINT,        MoveId::HYPERSPACEFURY, MoveId::HYPERSPACEHOLE,
      MoveId::PHANTOMFORCE, MoveId::SHADOWFORCE,
  };
  return std::find(listOfMoves.begin(), listOfMoves.end(), id) != listOfMoves.end();
}
// Could be 2-5 or strictly 2/3-hitting
bool MoveInstance::isMultiHit() {
  static const std::vector<MoveId> multihitMoves = {
      MoveId::ARMTHRUST,   MoveId::BARRAGE,        MoveId::BONERUSH,       MoveId::BULLETSEED,
      MoveId::COMETPUNCH,  MoveId::DOUBLESLAP,     MoveId::FURYATTACK,     MoveId::FURYSWIPES,
      MoveId::ICICLESPEAR, MoveId::PINMISSILE,     MoveId::ROCKBLAST,      MoveId::SCALESHOT,
      MoveId::SPIKECANNON, MoveId::TAILSLAP,       MoveId::WATERSHURIKEN,  MoveId::BONEMERANG,
      MoveId::DOUBLEHIT,   MoveId::DOUBLEIRONBASH, MoveId::DOUBLEKICK,     MoveId::DRAGONDARTS,
      MoveId::DUALCHOP,    MoveId::DUALWINGBEAT,   MoveId::GEARGRIND,      MoveId::TACHYONCUTTER,
      MoveId::TWINBEAM,    MoveId::TWINEEDLE,      MoveId::SURGINGSTRIKES, MoveId::TRIPLEAXEL,
      MoveId::TRIPLEDIVE,  MoveId::TRIPLEKICK,
  };
  return std::find(multihitMoves.begin(), multihitMoves.end(), id) != multihitMoves.end();
}
// Explicit exceptions
bool MoveInstance::isNoParentalBond() {
  static const std::vector<MoveId> no_pb_moves = {
      MoveId::DRAGONDARTS, MoveId::ENDEAVOR, MoveId::EXPLOSION, MoveId::FINALGAMBIT,
      MoveId::FLING,       MoveId::ICEBALL,  MoveId::ROLLOUT,   MoveId::SELFDESTRUCT,
  };
  return std::find(no_pb_moves.begin(), no_pb_moves.end(), id) != no_pb_moves.end();
}
// For checking Powder immunity
bool MoveInstance::isPowder() {
  static const std::vector<MoveId> powderMoves = {
      MoveId::COTTONSPORE, MoveId::MAGICPOWDER, MoveId::POISONPOWDER, MoveId::POWDER,
      MoveId::RAGEPOWDER,  MoveId::SLEEPPOWDER, MoveId::SPORE,        MoveId::STUNSPORE};
  return std::find(powderMoves.begin(), powderMoves.end(), id) != powderMoves.end();
}
// Two-turn or SkyDrop
bool MoveInstance::isCharge() {
  static const std::vector<MoveId> chargeMoves = {
      MoveId::BOUNCE,     MoveId::DIG,          MoveId::DIVE,      MoveId::ELECTROSHOT,
      MoveId::FLY,        MoveId::FREEZESHOCK,  MoveId::GEOMANCY,  MoveId::ICEBURN,
      MoveId::METEORBEAM, MoveId::PHANTOMFORCE, MoveId::RAZORWIND, MoveId::SHADOWFORCE,
      MoveId::SKULLBASH,  MoveId::SKYATTACK,    MoveId::SKYDROP,   MoveId::SOLARBEAM,
      MoveId::SOLARBLADE,
  };
  return std::find(chargeMoves.begin(), chargeMoves.end(), id) != chargeMoves.end();
}
// Regular recoil moves
std::pair<int, int> MoveInstance::getRecoil() {
  static std::unordered_map<MoveId, std::pair<int, int>> recoilMoves = {
      {MoveId::BRAVEBIRD, {33, 100}},  {MoveId::DOUBLEEDGE, {33, 100}},
      {MoveId::FLAREBLITZ, {33, 100}}, {MoveId::VOLTTACKLE, {33, 100}},
      {MoveId::WAVECRASH, {33, 100}},  {MoveId::WOODHAMMER, {33, 100}},
      {MoveId::HEADSMASH, {1, 2}},     {MoveId::LIGHTOFRUIN, {1, 2}},
      {MoveId::HEADCHARGE, {1, 4}},    {MoveId::SUBMISSION, {1, 4}},
      {MoveId::TAKEDOWN, {1, 4}},      {MoveId::WILDCHARGE, {1, 4}},
  };
  auto it = recoilMoves.find(id);
  return it != recoilMoves.end() ? it->second : std::make_pair(0, 0);
}
std::vector<SecondaryEffect> MoveInstance::getSecondaries() {
  std::vector<SecondaryEffect> ret;
  // --------- STATUS ---------
  switch (id) {
  // Burn
  case MoveId::BLAZEKICK:
  case MoveId::EMBER:
  case MoveId::FIREBLAST:
  case MoveId::FIREPUNCH:
  case MoveId::FLAMETHROWER:
  case MoveId::FLAMEWHEEL:
  case MoveId::FLAREBLITZ:
  case MoveId::HEATWAVE:
  case MoveId::PYROBALL: {
    ret.push_back({.chance = 10, .status = Status::BURN});
    break;
  }
  case MoveId::BLUEFLARE:
  case MoveId::MATCHAGOTCHA:
  case MoveId::SANDSEARSTORM: {
    ret.push_back({.chance = 20, .status = Status::BURN});
    break;
  }
  case MoveId::BLAZINGTORQUE:
  case MoveId::ICEBURN:
  case MoveId::INFERNALPARADE:
  case MoveId::LAVAPLUME:
  case MoveId::SCALD:
  case MoveId::SCORCHINGSANDS:
  case MoveId::SEARINGSHOT:
  case MoveId::STEAMERUPTION: {
    ret.push_back({.chance = 30, .status = Status::BURN});
    break;
  }
  case MoveId::SACREDFIRE: {
    ret.push_back({.chance = 50, .status = Status::BURN});
    break;
  }
  case MoveId::INFERNO: {
    ret.push_back({.chance = 100, .status = Status::BURN});
    break;
  }
  // Freeze
  case MoveId::BLIZZARD:
  case MoveId::FREEZEDRY:
  case MoveId::FREEZINGGLARE:
  case MoveId::ICEBEAM:
  case MoveId::ICEPUNCH:
  case MoveId::POWDERSNOW: {
    ret.push_back({.chance = 10, .status = Status::FREEZE});
    break;
  }
  // Paralysis
  case MoveId::THUNDERBOLT:
  case MoveId::THUNDERPUNCH:
  case MoveId::THUNDERSHOCK:
  case MoveId::VOLTTACKLE: {
    ret.push_back({.chance = 10, .status = Status::PARALYSIS});
    break;
  }
  case MoveId::BOLTSTRIKE:
  case MoveId::WILDBOLTSTORM: {
    ret.push_back({.chance = 20, .status = Status::PARALYSIS});
    break;
  }
  case MoveId::BODYSLAM:
  case MoveId::BOUNCE:
  case MoveId::COMBATTORQUE:
  case MoveId::DISCHARGE:
  case MoveId::DRAGONBREATH:
  case MoveId::FORCEPALM:
  case MoveId::FREEZESHOCK:
  case MoveId::LICK:
  case MoveId::SECRETPOWER:
  case MoveId::SPARK:
  case MoveId::THUNDER: {
    ret.push_back({.chance = 30, .status = Status::PARALYSIS});
    break;
  }
  case MoveId::NUZZLE:
  case MoveId::ZAPCANNON: {
    ret.push_back({.chance = 100, .status = Status::PARALYSIS});
    break;
  }
  // Poison
  case MoveId::CROSSPOISON:
  case MoveId::POISONTAIL:
  case MoveId::SLUDGEWAVE: {
    ret.push_back({.chance = 10, .status = Status::POISON});
    break;
  }
  case MoveId::SHELLSIDEARM:
  case MoveId::TWINEEDLE: {
    ret.push_back({.chance = 20, .status = Status::POISON});
    break;
  }
  case MoveId::GUNKSHOT:
  case MoveId::NOXIOUSTORQUE:
  case MoveId::POISONJAB:
  case MoveId::POISONSTING:
  case MoveId::SLUDGE:
  case MoveId::SLUDGEBOMB: {
    ret.push_back({.chance = 30, .status = Status::POISON});
    break;
  }
  case MoveId::SMOG: {
    ret.push_back({.chance = 40, .status = Status::POISON});
    break;
  }
  case MoveId::BARBBARRAGE: {
    ret.push_back({.chance = 50, .status = Status::POISON});
    break;
  }
  case MoveId::MORTALSPIN: {
    ret.push_back({.chance = 100, .status = Status::POISON});
    break;
  }
  // Sleep
  case MoveId::RELICSONG:
  case MoveId::WICKEDTORQUE: {
    ret.push_back({.chance = 10, .status = Status::SLEEP});
    break;
  }
  // Toxic
  case MoveId::MALIGNANTCHAIN:
  case MoveId::POISONFANG: {
    ret.push_back({.chance = 50, .status = Status::TOXIC});
    break;
  }
  default:
    break;
  }
  // --------- BOOSTS ---------
  switch (id) {
    // 2x Special Defense
  case MoveId::ACIDSPRAY:
  case MoveId::LUMINACRASH: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 100, .boosts = {{ModifierId::SPDEF, -2}}});
    break;
  }
  case MoveId::SEEDFLARE: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 40, .boosts = {{ModifierId::SPDEF, -2}}});
    break;
  }
  // Accuracy
  case MoveId::MIRRORSHOT:
  case MoveId::MUDBOMB:
  case MoveId::MUDDYWATER: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 30, .boosts = {{ModifierId::ACCURACY, -1}}});
    break;
  }
  case MoveId::NIGHTDAZE: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 40, .boosts = {{ModifierId::ACCURACY, -1}}});
    break;
  }
  case MoveId::LEAFTORNADO:
  case MoveId::OCTAZOOKA: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 50, .boosts = {{ModifierId::ACCURACY, -1}}});
    break;
  }
  case MoveId::MUDSLAP: {
    ret.push_back(
        {.kind = Secondary::BOOST, .chance = 100, .boosts = {{ModifierId::ACCURACY, -1}}});
    break;
  }
  // Attack
  case MoveId::AURORABEAM:
  case MoveId::PLAYROUGH: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 10, .boosts = {{ModifierId::ATTACK, -1}}});
    break;
  }
  case MoveId::SPRINGTIDESTORM: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 30, .boosts = {{ModifierId::ATTACK, -1}}});
    break;
  }
  case MoveId::BITTERMALICE:
  case MoveId::BREAKINGSWIPE:
  case MoveId::CHILLINGWATER:
  case MoveId::LUNGE:
  case MoveId::TROPKICK: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 100, .boosts = {{ModifierId::ATTACK, -1}}});
    break;
  }
  // Defense
  case MoveId::CRUNCH:
  case MoveId::LIQUIDATION:
  case MoveId::SHADOWBONE: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 20, .boosts = {{ModifierId::DEFENSE, -1}}});
    break;
  }
  case MoveId::IRONTAIL: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 30, .boosts = {{ModifierId::DEFENSE, -1}}});
    break;
  }
  case MoveId::CRUSHCLAW:
  case MoveId::RAZORSHELL:
  case MoveId::ROCKSMASH: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 50, .boosts = {{ModifierId::DEFENSE, -1}}});
    break;
  }
  case MoveId::FIRELASH:
  case MoveId::GRAVAPPLE:
  case MoveId::THUNDEROUSKICK: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 100, .boosts = {{ModifierId::DEFENSE, -1}}});
    break;
  }
  // Special Attack
  case MoveId::MOONBLAST: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 30, .boosts = {{ModifierId::SPATT, -1}}});
    break;
  }
  case MoveId::MISTBALL: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 50, .boosts = {{ModifierId::SPATT, -1}}});
    break;
  }
  case MoveId::MYSTICALFIRE:
  case MoveId::SKITTERSMACK:
  case MoveId::SNARL:
  case MoveId::SPIRITBREAK:
  case MoveId::STRUGGLEBUG: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 100, .boosts = {{ModifierId::SPATT, -1}}});
    break;
  }
  // Special Defense
  case MoveId::ACID:
  case MoveId::BUGBUZZ:
  case MoveId::EARTHPOWER:
  case MoveId::ENERGYBALL:
  case MoveId::FLASHCANNON:
  case MoveId::FOCUSBLAST:
  case MoveId::PSYCHIC: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 10, .boosts = {{ModifierId::SPDEF, -1}}});
    break;
  }
  case MoveId::SHADOWBALL: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 20, .boosts = {{ModifierId::SPDEF, -1}}});
    break;
  }
  case MoveId::LUSTERPURGE: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 50, .boosts = {{ModifierId::SPDEF, -1}}});
    break;
  }
  case MoveId::APPLEACID: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 100, .boosts = {{ModifierId::SPDEF, -1}}});
    break;
  }
  // Speed
  case MoveId::BUBBLE:
  case MoveId::BUBBLEBEAM:
  case MoveId::CONSTRICT: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 10, .boosts = {{ModifierId::SPEED, -1}}});
    break;
  }
  case MoveId::BLEAKWINDSTORM: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 30, .boosts = {{ModifierId::SPEED, -1}}});
    break;
  }
  case MoveId::BULLDOZE:
  case MoveId::DRUMBEATING:
  case MoveId::ELECTROWEB:
  case MoveId::GLACIATE:
  case MoveId::ICYWIND:
  case MoveId::LOWSWEEP:
  case MoveId::MUDSHOT:
  case MoveId::POUNCE:
  case MoveId::ROCKTOMB: {
    ret.push_back({.kind = Secondary::BOOST, .chance = 100, .boosts = {{ModifierId::SPEED, -1}}});
    break;
  }
  default:
    break;
  }
  // --------- SELFBOOSTS ---------
  switch (id) {
  // All (Attack, Defense, SpAtt, SpDef, Speed)
  case MoveId::ANCIENTPOWER:
  case MoveId::OMINOUSWIND:
  case MoveId::SILVERWIND: {
    ret.push_back({.kind = Secondary::SELFBOOST,
                   .boosts = {{ModifierId::ATTACK, 1},
                              {ModifierId::DEFENSE, 1},
                              {ModifierId::SPATT, 1},
                              {ModifierId::SPDEF, 1},
                              {ModifierId::SPEED, 1}}});
    break;
  }
  // Attack
  case MoveId::METALCLAW:
  case MoveId::METEORMASH:
  case MoveId::POWERUPPUNCH: {
    ret.push_back({.kind = Secondary::SELFBOOST, .boosts = {{ModifierId::ATTACK, 1}}});
    break;
  }
  // Defense
  case MoveId::PSYSHIELDBASH:
  case MoveId::STEELWING: {
    ret.push_back({.kind = Secondary::SELFBOOST, .boosts = {{ModifierId::DEFENSE, 1}}});
    break;
  }
  case MoveId::DIAMONDSTORM: {
    ret.push_back(
        {.kind = Secondary::SELFBOOST, .chance = 50, .boosts = {{ModifierId::DEFENSE, 1}}});
    break;
  }
  // Special Attack
  case MoveId::CHARGEBEAM:
  case MoveId::FIERYDANCE:
  case MoveId::MYSTICALPOWER:
  case MoveId::TORCHSONG: {
    ret.push_back({.kind = Secondary::SELFBOOST, .boosts = {{ModifierId::SPATT, 1}}});
    break;
  }
  // Speed
  case MoveId::AQUASTEP:
  case MoveId::AURAWHEEL:
  case MoveId::ESPERWING:
  case MoveId::FLAMECHARGE:
  case MoveId::RAPIDSPIN:
  case MoveId::TRAILBLAZE: {
    ret.push_back({.kind = Secondary::SELFBOOST, .boosts = {{ModifierId::SPEED, 1}}});
    break;
  }
  default:
    break;
  }
  // --------- VOLATILES ---------
  switch (id) {
  // Confusion
  case MoveId::CONFUSION:
  case MoveId::PSYBEAM:
  case MoveId::SIGNALBEAM: {
    ret.push_back({.chance = 10, .vol = VolatileId::CONFUSION});
    break;
  }
  case MoveId::DIZZYPUNCH:
  case MoveId::ROCKCLIMB:
  case MoveId::STRANGESTEAM:
  case MoveId::WATERPULSE: {
    ret.push_back({.chance = 20, .vol = VolatileId::CONFUSION});
    break;
  }
  case MoveId::AXEKICK:
  case MoveId::HURRICANE:
  case MoveId::MAGICALTORQUE: {
    ret.push_back({.chance = 30, .vol = VolatileId::CONFUSION});
    break;
  }
  case MoveId::CHATTER:
  case MoveId::DYNAMICPUNCH: {
    ret.push_back({.chance = 100, .vol = VolatileId::CONFUSION});
    break;
  }
  // Flinch
  case MoveId::BONECLUB:
  case MoveId::EXTRASENSORY:
  case MoveId::HYPERFANG: {
    ret.push_back({.chance = 10, .vol = VolatileId::FLINCH});
    break;
  }
  case MoveId::DARKPULSE:
  case MoveId::DRAGONRUSH:
  case MoveId::FIERYWRATH:
  case MoveId::TWISTER:
  case MoveId::WATERFALL:
  case MoveId::ZENHEADBUTT: {
    ret.push_back({.chance = 20, .vol = VolatileId::FLINCH});
    break;
  }
  case MoveId::AIRSLASH:
  case MoveId::ASTONISH:
  case MoveId::BITE:
  case MoveId::DOUBLEIRONBASH:
  case MoveId::HEADBUTT:
  case MoveId::HEARTSTAMP:
  case MoveId::ICICLECRASH:
  case MoveId::IRONHEAD:
  case MoveId::MOUNTAINGALE:
  case MoveId::NEEDLEARM:
  case MoveId::ROCKSLIDE:
  case MoveId::ROLLINGKICK:
  case MoveId::SKYATTACK:
  case MoveId::SNORE:
  case MoveId::STEAMROLLER:
  case MoveId::STOMP:
  case MoveId::ZINGZAP: {
    ret.push_back({.chance = 30, .vol = VolatileId::FLINCH});
    break;
  }
  case MoveId::FAKEOUT:
  case MoveId::UPPERHAND: {
    ret.push_back({.chance = 100, .vol = VolatileId::FLINCH});
    break;
  }
  // Move-specific
  case MoveId::PSYCHICNOISE: {
    ret.push_back({.chance = 100, .vol = VolatileId::HEAL_BLOCK});
    break;
  }
  case MoveId::SALTCURE: {
    ret.push_back({.chance = 100, .vol = VolatileId::SALT_CURE});
    break;
  }
  case MoveId::SPARKLINGARIA: {
    ret.push_back({.chance = 100, .vol = VolatileId::SPARKLING_ARIA});
    break;
  }
  default:
    break;
  }
  // --------- CALLBACKS ---------
  switch (id) {
  case MoveId::ALLURINGVOICE:
  case MoveId::ANCHORSHOT:
  case MoveId::BURNINGJEALOUSY:
  case MoveId::DIRECLAW:
  case MoveId::EERIESPELL:
  case MoveId::SPIRITSHACKLE:
  case MoveId::THROATCHOP:
  case MoveId::TRIATTACK: {
    ret.push_back({.kind = Secondary::CALLBACK});
    break;
  }
  default:
    break;
  }
  // --------- SHEERFORCEBOOST ---------
  switch (id) {
  case MoveId::CEASELESSEDGE:
  case MoveId::DIAMONDSTORM:
  case MoveId::STONEAXE: {
    ret.push_back({.kind = Secondary::SHEERFORCEBOOST});
    break;
  }
  default:
    break;
  }
  return ret;
}
SecondaryEffect MoveInstance::getSelfEffect() {
  // Applying volatiles
  switch (id) {
  // Recharge
  case MoveId::BLASTBURN:
  case MoveId::ETERNABEAM:
  case MoveId::FRENZYPLANT:
  case MoveId::GIGAIMPACT:
  case MoveId::HYDROCANNON:
  case MoveId::HYPERBEAM:
  case MoveId::METEORASSAULT:
  case MoveId::PRISMATICLASER:
  case MoveId::ROAROFTIME:
  case MoveId::ROCKWRECKER: {
    return {.kind = Secondary::VOLATILE, .vol = VolatileId::MUST_RECHARGE};
  }
  case MoveId::OUTRAGE:
  case MoveId::PETALDANCE:
  case MoveId::RAGINGFURY:
  case MoveId::THRASH: {
    return {.kind = Secondary::VOLATILE, .vol = VolatileId::LOCKED_MOVE};
  }
  // Move-specific
  case MoveId::RAGE: {
    return {.kind = Secondary::VOLATILE, .vol = VolatileId::RAGE};
  }
  case MoveId::ROOST: {
    return {.kind = Secondary::VOLATILE, .vol = VolatileId::ROOST};
  }
  case MoveId::UPROAR: {
    return {.kind = Secondary::VOLATILE, .vol = VolatileId::UPROAR};
  }
  default: break;
  }
  return {};
}
// DoomDesire and FutureSight
bool MoveInstance::isFuture() { return id == MoveId::DOOMDESIRE || id == MoveId::FUTURESIGHT; }
// Calls another move, e.g. Assist
bool MoveInstance::isCalling() {
  static const std::vector<MoveId> callMoves = {
      MoveId::ASSIST,     MoveId::COPYCAT,     MoveId::MEFIRST,   MoveId::METRONOME,
      MoveId::MIRRORMOVE, MoveId::NATUREPOWER, MoveId::SLEEPTALK,
  };
  return std::find(callMoves.begin(), callMoves.end(), id) != callMoves.end();
}
// Self-explanatory
bool MoveInstance::isOHKO() {
  // TODO: have special case for Ice-type!
  static const std::vector<MoveId> ohkos = {MoveId::FISSURE, MoveId::GUILLOTINE, MoveId::HORNDRILL,
                                            MoveId::SHEERCOLD};
  return std::find(ohkos.begin(), ohkos.end(), id) != ohkos.end();
}
// Self-explanatory
bool MoveInstance::isSleepUsable() { return id == MoveId::SLEEPTALK || id == MoveId::SNORE; }
// Only self-destructs if the move "hits"
bool MoveInstance::isIffHitSelfDestruct() {
  static const std::vector<MoveId> ifHitSelfDestructing = {MoveId::FINALGAMBIT, MoveId::HEALINGWISH,
                                                           MoveId::LUNARDANCE, MoveId::MEMENTO};
  return std::find(ifHitSelfDestructing.begin(), ifHitSelfDestructing.end(), id) !=
         ifHitSelfDestructing.end();
}
// Upon successful usage, switches the user out.
bool MoveInstance::isSelfSwitch() {
  // Note/Do later: BatonPass->'copyvolatile', ShedTail->'shedtail'
  static const std::vector<MoveId> selfSwitching = {
      MoveId::BATONPASS, MoveId::CHILLYRECEPTION, MoveId::FLIPTURN, MoveId::PARTINGSHOT,
      MoveId::SHEDTAIL,  MoveId::TELEPORT,        MoveId::UTURN,    MoveId::VOLTSWITCH};
  return std::find(selfSwitching.begin(), selfSwitching.end(), id) != selfSwitching.end();
}
// Drags out target
bool MoveInstance::forcesSwitch() {
  static const std::vector<MoveId> forceSwitch = {
      MoveId::CIRCLETHROW,
      MoveId::DRAGONTAIL,
      MoveId::ROAR,
      MoveId::WHIRLWIND,
  };
  return std::find(forceSwitch.begin(), forceSwitch.end(), id) != forceSwitch.end();
}
bool MoveInstance::makesContact(Pokemon &user) {
  static const std::unordered_set<MoveId> contactMoves = {
      MoveId::ACCELEROCK,
      MoveId::ACROBATICS,
      MoveId::AERIALACE,
      MoveId::ANCHORSHOT,
      MoveId::AQUAJET,
      MoveId::AQUASTEP,
      MoveId::AQUATAIL,
      MoveId::ARMTHRUST,
      MoveId::ASSURANCE,
      MoveId::ASTONISH,
      MoveId::AVALANCHE,
      MoveId::AXEKICK,
      MoveId::BEHEMOTHBASH,
      MoveId::BEHEMOTHBLADE,
      MoveId::BIDE,
      MoveId::BIND,
      MoveId::BITE,
      MoveId::BITTERBLADE,
      MoveId::BLAZEKICK,
      MoveId::BODYPRESS,
      MoveId::BODYSLAM,
      MoveId::BOLTBEAK,
      MoveId::BOLTSTRIKE,
      MoveId::BOUNCE,
      MoveId::BRANCHPOKE,
      MoveId::BRAVEBIRD,
      MoveId::BREAKINGSWIPE,
      MoveId::BRICKBREAK,
      MoveId::BRUTALSWING,
      MoveId::BUGBITE,
      MoveId::BULLETPUNCH,
      MoveId::CEASELESSEDGE,
      MoveId::CHIPAWAY,
      MoveId::CIRCLETHROW,
      MoveId::CLAMP,
      MoveId::CLOSECOMBAT,
      MoveId::COLLISIONCOURSE,
      MoveId::COMETPUNCH,
      MoveId::COMEUPPANCE,
      MoveId::CONSTRICT,
      MoveId::COUNTER,
      MoveId::COVET,
      MoveId::CRABHAMMER,
      MoveId::CROSSCHOP,
      MoveId::CROSSPOISON,
      MoveId::CRUNCH,
      MoveId::CRUSHCLAW,
      MoveId::CRUSHGRIP,
      MoveId::CUT,
      MoveId::DARKESTLARIAT,
      MoveId::DIG,
      MoveId::DIRECLAW,
      MoveId::DIVE,
      MoveId::DIZZYPUNCH,
      MoveId::DOUBLEEDGE,
      MoveId::DOUBLEHIT,
      MoveId::DOUBLEIRONBASH,
      MoveId::DOUBLEKICK,
      MoveId::DOUBLESHOCK,
      MoveId::DOUBLESLAP,
      MoveId::DRAGONASCENT,
      MoveId::DRAGONCLAW,
      MoveId::DRAGONHAMMER,
      MoveId::DRAGONRUSH,
      MoveId::DRAGONTAIL,
      MoveId::DRAININGKISS,
      MoveId::DRAINPUNCH,
      MoveId::DRILLPECK,
      MoveId::DRILLRUN,
      MoveId::DUALCHOP,
      MoveId::DUALWINGBEAT,
      MoveId::DYNAMICPUNCH,
      MoveId::ELECTRODRIFT,
      MoveId::ENDEAVOR,
      MoveId::EXTREMESPEED,
      MoveId::FACADE,
      MoveId::FAKEOUT,
      MoveId::FALSESURRENDER,
      MoveId::FALSESWIPE,
      MoveId::FEINTATTACK,
      MoveId::FELLSTINGER,
      MoveId::FIREFANG,
      MoveId::FIRELASH,
      MoveId::FIREPUNCH,
      MoveId::FIRSTIMPRESSION,
      MoveId::FISHIOUSREND,
      MoveId::FLAIL,
      MoveId::FLAMECHARGE,
      MoveId::FLAMEWHEEL,
      MoveId::FLAREBLITZ,
      MoveId::FLIPTURN,
      MoveId::FLY,
      MoveId::FLYINGPRESS,
      MoveId::FOCUSPUNCH,
      MoveId::FORCEPALM,
      MoveId::FOULPLAY,
      MoveId::FRUSTRATION,
      MoveId::FURYATTACK,
      MoveId::FURYCUTTER,
      MoveId::FURYSWIPES,
      MoveId::GEARGRIND,
      MoveId::GIGAIMPACT,
      MoveId::GLAIVERUSH,
      MoveId::GRASSKNOT,
      MoveId::GRASSYGLIDE,
      MoveId::GUILLOTINE,
      MoveId::GYROBALL,
      MoveId::HAMMERARM,
      MoveId::HARDPRESS,
      MoveId::HEADBUTT,
      MoveId::HEADCHARGE,
      MoveId::HEADLONGRUSH,
      MoveId::HEADSMASH,
      MoveId::HEARTSTAMP,
      MoveId::HEATCRASH,
      MoveId::HEAVYSLAM,
      MoveId::HIGHHORSEPOWER,
      MoveId::HIGHJUMPKICK,
      MoveId::HOLDBACK,
      MoveId::HORNATTACK,
      MoveId::HORNDRILL,
      MoveId::HORNLEECH,
      MoveId::HYPERDRILL,
      MoveId::HYPERFANG,
      MoveId::ICEBALL,
      MoveId::ICEFANG,
      MoveId::ICEHAMMER,
      MoveId::ICEPUNCH,
      MoveId::ICESPINNER,
      MoveId::INFESTATION,
      MoveId::IRONHEAD,
      MoveId::IRONTAIL,
      MoveId::JAWLOCK,
      MoveId::JETPUNCH,
      MoveId::JUMPKICK,
      MoveId::KARATECHOP,
      MoveId::KNOCKOFF,
      MoveId::KOWTOWCLEAVE,
      MoveId::LASHOUT,
      MoveId::LASTRESORT,
      MoveId::LEAFBLADE,
      MoveId::LEECHLIFE,
      MoveId::LICK,
      MoveId::LIQUIDATION,
      MoveId::LOWKICK,
      MoveId::LOWSWEEP,
      MoveId::LUNGE,
      MoveId::MACHPUNCH,
      MoveId::MEGAHORN,
      MoveId::MEGAKICK,
      MoveId::MEGAPUNCH,
      MoveId::METALCLAW,
      MoveId::METEORMASH,
      MoveId::MIGHTYCLEAVE,
      MoveId::MORTALSPIN,
      MoveId::MULTIATTACK,
      MoveId::NEEDLEARM,
      MoveId::NIGHTSLASH,
      MoveId::NUZZLE,
      MoveId::OUTRAGE,
      MoveId::PAYBACK,
      MoveId::PECK,
      MoveId::PETALDANCE,
      MoveId::PHANTOMFORCE,
      MoveId::PLASMAFISTS,
      MoveId::PLAYROUGH,
      MoveId::PLUCK,
      MoveId::POISONFANG,
      MoveId::POISONJAB,
      MoveId::POISONTAIL,
      MoveId::POPULATIONBOMB,
      MoveId::POUNCE,
      MoveId::POUND,
      MoveId::POWERTRIP,
      MoveId::POWERUPPUNCH,
      MoveId::POWERWHIP,
      MoveId::PSYBLADE,
      MoveId::PSYCHICFANGS,
      MoveId::PSYSHIELDBASH,
      MoveId::PUNISHMENT,
      MoveId::PURSUIT,
      MoveId::QUICKATTACK,
      MoveId::RAGE,
      MoveId::RAGEFIST,
      MoveId::RAGINGBULL,
      MoveId::RAPIDSPIN,
      MoveId::RAZORSHELL,
      MoveId::RETALIATE,
      MoveId::RETURN,
      MoveId::REVENGE,
      MoveId::REVERSAL,
      MoveId::ROCKCLIMB,
      MoveId::ROCKSMASH,
      MoveId::ROLLINGKICK,
      MoveId::ROLLOUT,
      MoveId::SACREDSWORD,
      MoveId::SCRATCH,
      MoveId::SEISMICTOSS,
      MoveId::SHADOWCLAW,
      MoveId::SHADOWFORCE,
      MoveId::SHADOWPUNCH,
      MoveId::SHADOWSNEAK,
      MoveId::SKITTERSMACK,
      MoveId::SKULLBASH,
      MoveId::SKYDROP,
      MoveId::SKYUPPERCUT,
      MoveId::SLAM,
      MoveId::SLASH,
      MoveId::SMARTSTRIKE,
      MoveId::SMELLINGSALTS,
      MoveId::SNAPTRAP,
      MoveId::SOLARBLADE,
      MoveId::SPARK,
      MoveId::SPECTRALTHIEF,
      MoveId::SPINOUT,
      MoveId::SPIRITBREAK,
      MoveId::STEAMROLLER,
      MoveId::STEELROLLER,
      MoveId::STEELWING,
      MoveId::STOMP,
      MoveId::STOMPINGTANTRUM,
      MoveId::STONEAXE,
      MoveId::STORMTHROW,
      MoveId::STRENGTH,
      MoveId::STRUGGLE,
      MoveId::SUBMISSION,
      MoveId::SUCKERPUNCH,
      MoveId::SUNSTEELSTRIKE,
      MoveId::SUPERCELLSLAM,
      MoveId::SUPERFANG,
      MoveId::SUPERPOWER,
      MoveId::SURGINGSTRIKES,
      MoveId::TACKLE,
      MoveId::TAILSLAP,
      MoveId::TAKEDOWN,
      MoveId::TEMPERFLARE,
      MoveId::THIEF,
      MoveId::THRASH,
      MoveId::THROATCHOP,
      MoveId::THUNDERFANG,
      MoveId::THUNDEROUSKICK,
      MoveId::THUNDERPUNCH,
      MoveId::TRAILBLAZE,
      MoveId::TRIPLEAXEL,
      MoveId::TRIPLEDIVE,
      MoveId::TRIPLEKICK,
      MoveId::TROPKICK,
      MoveId::TRUMPCARD,
      MoveId::UTURN,
      MoveId::UPPERHAND,
      MoveId::VCREATE,
      MoveId::VINEWHIP,
      MoveId::VISEGRIP,
      MoveId::VITALTHROW,
      MoveId::VOLTTACKLE,
      MoveId::WAKEUPSLAP,
      MoveId::WATERFALL,
      MoveId::WAVECRASH,
      MoveId::WICKEDBLOW,
      MoveId::WILDCHARGE,
      MoveId::WINGATTACK,
      MoveId::WOODHAMMER,
      MoveId::WRAP,
      MoveId::WRINGOUT,
      MoveId::XSCISSOR,
      MoveId::ZENHEADBUTT,
      MoveId::ZINGZAP,
  };
  return std::find(contactMoves.begin(), contactMoves.end(), id) != contactMoves.end() ||
         user.has_item(Item::PROTECTIVE_PADS);
}
// Compute numHits, with Skill Link and Loaded Dice potential logic.
// Without PopulationBomb or BeatUp, every 2-5 has the same randomization.
// Accuracy check type defined by move.multiaccuracy.
int MoveInstance::getNumHits(Pokemon const &pokemon) {
  // Moves that hit 2-5 times.
  static const std::vector<MoveId> hit_2_5{
      MoveId::ARMTHRUST,   MoveId::BARRAGE,    MoveId::BONERUSH,     MoveId::BULLETSEED,
      MoveId::COMETPUNCH,  MoveId::DOUBLESLAP, MoveId::FURYATTACK,   MoveId::FURYSWIPES,
      MoveId::ICICLESPEAR, MoveId::PINMISSILE, MoveId::ROCKBLAST,    MoveId::SCALESHOT,
      MoveId::SPIKECANNON, MoveId::TAILSLAP,   MoveId::WATERSHURIKEN};
  // Moves that hit twice.
  static const std::vector<MoveId> hit_2{
      MoveId::BONEMERANG,    MoveId::DOUBLEHIT, MoveId::DOUBLEIRONBASH, MoveId::DOUBLEKICK,
      MoveId::DRAGONDARTS,   MoveId::DUALCHOP,  MoveId::DUALWINGBEAT,   MoveId::GEARGRIND,
      MoveId::TACHYONCUTTER, MoveId::TWINBEAM,  MoveId::TWINEEDLE};
  // Moves that hit three times.
  static const std::vector<MoveId> hit_3{MoveId::SURGINGSTRIKES, MoveId::TRIPLEAXEL,
                                         MoveId::TRIPLEDIVE, MoveId::TRIPLEKICK};
  // Population bomb is a special case.
  if (std::find(hit_2.begin(), hit_2.end(), id) != hit_2.end()) {
    return 2;
  }
  if (std::find(hit_3.begin(), hit_3.end(), id) != hit_3.end()) {
    return 3;
  }
  if (std::find(hit_2_5.begin(), hit_2_5.end(), id) != hit_2_5.end()) {
    // 35-35-15-15 out of 100 for 2-3-4-5 hits
    double r = math::random();
    int numHits = 2;
    if (r > 0.35) {
      numHits++;
    }
    if (r > 0.7) {
      numHits++;
    }
    if (r > 0.85) {
      numHits++;
    }
    if (numHits < 4 && pokemon.has_item(Item::LOADED_DICE)) {
      return 5 - math::random(2);
    }
  }
  return 1;
}
// "multiaccuracy" moves: Population Bomb, Triple Axel, and Triple Kick.
// These moves run accuracy checks on *every* hit.
// The Loaded Dice item and Skill Link ability turns it off (one acc check).
bool MoveInstance::multiaccCheck(Pokemon const &user) {
  static const std::vector<MoveId> multiacc_moves{MoveId::POPULATIONBOMB, MoveId::TRIPLEAXEL,
                                                  MoveId::TRIPLEKICK};
  bool multiaccuracy =
      std::find(multiacc_moves.begin(), multiacc_moves.end(), id) != multiacc_moves.end();
  if (user.has_item(Item::LOADED_DICE) || user.has_ability(Ability::SKILL_LINK)) {
    multiaccuracy = false;
  }
  return multiaccuracy;
}
// Checks relevant Accuracy and Evasion boosts and onAccuracy (e.g.
// conditions like Sandstorm+SandVeil or Gravity).
// **Assumes the move has numerical accuracy -- do acc check bypasses first!
int MoveInstance::getBasicAcc(int acc, bool multiacc, Pokemon const &target,
                              Pokemon const &pokemon) {
  // Accuracy boosts
  int accBoost = std::min(6, std::max(-6, pokemon.boosts.acc));
  int evaBoost = 0;
  if (!(target.foresight || target.miracleeye) || target.boosts.eva <= 0) {
    evaBoost = std::min(6, std::max(-6, target.boosts.eva));
  }
  if (multiacc) {
    if (accBoost > 0) {
      acc *= (3 + accBoost) / 3.;
    } else if (accBoost < 0) {
      acc /= (3 - accBoost) / 3.;
    }
    if (evaBoost > 0) {
      acc /= (3 + evaBoost) / 3.;
    } else if (evaBoost < 0) {
      acc *= (3 - evaBoost) / 3.;
    }
  } else {
    int boost = accBoost - evaBoost;
    if (boost > 0) {
      acc *= (3 + boost) / 3.;
    } else if (boost < 0) {
      acc *= 3. / (3 - boost);
    }
  }
  return acc;
}
} // namespace pkmn