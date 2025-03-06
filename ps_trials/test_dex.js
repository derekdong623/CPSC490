const fs = require('fs');
const {Dex} = require('pokemon-showdown')
Dex.mod('gen8')
// // for(const format of Dex.formats.formatsListCache) {
// //   if(format.id === "gen8anythinggoes") {
// //     console.log(format)
// //   }
// // }
// console.log(Dex.moves.get('counter'))
const replacer = (key, value) => {
  if (value === undefined) return null;
  if (typeof value === "function") return value.name || "[anonymous function]";
  return value;
}
// console.log(JSON.stringify(Dex.moves.get('counter'), replacer))
fs.writeFileSync('move_data.json', JSON.stringify(Dex.moves.all(), replacer), 'utf8');

// console.log(Dex.moves.get('poisongas').ignoreImmunity)
// console.log(Dex.moves.get('poisonpowder').ignoreImmunity)
// console.log(Dex.moves.get('poisonfang').ignoreImmunity)
// console.log(Dex.moves.get('counter').ignoreImmunity)
// // console.log(Dex.moves.get('auroraveil').condition.onAnyModifyDamage.toString())