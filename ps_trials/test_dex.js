const fs = require('fs');
const {Dex} = require('pokemon-showdown')
Dex.mod('gen8')
// // for(const format of Dex.formats.formatsListCache) {
// //   if(format.id === "gen8anythinggoes") {
// //     console.log(format)
// //   }
// // }
// fs.writeFileSync('items_list.txt', '', 'utf8')
// for(item of Dex.items.all()) {
//   fs.appendFileSync('items_list.txt', item.name.replace(/ /g, "_").replace("(", "").replace(")", "").replace("'", "").replace('-', '_').toUpperCase() + ',\n', 'utf8')
// }
// fs.writeFileSync('abilities_list.txt', '', 'utf8')
// for(ability of Dex.abilities.all()) {
//   fs.appendFileSync('abilities_list.txt', ability.name.replace(/ /g, "_").replace("(", "").replace(")", "").replace("'", "").replace('-', '_').toUpperCase() + ',\n', 'utf8')
// }
// const moves_replacer = (key, value) => {
//   if (value === undefined) return null;
//   if (typeof value === "function") return value.name || "[anonymous function]";
//   return value;
// }
// console.log(JSON.stringify(Dex.moves.get('counter'), replacer))
// fs.writeFileSync('move_data.json', JSON.stringify(Dex.moves.all(), moves_replacer), 'utf8');

// console.log(Dex.moves.get('poisongas').ignoreImmunity)
// console.log(Dex.moves.get('poisonpowder').ignoreImmunity)
// console.log(Dex.moves.get('poisonfang').ignoreImmunity)
// console.log(Dex.moves.get('counter').ignoreImmunity)
// // console.log(Dex.moves.get('auroraveil').condition.onAnyModifyDamage.toString())