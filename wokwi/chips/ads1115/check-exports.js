// check-exports.js
const fs = require('fs');

(async () => {
  const buf = fs.readFileSync('ads1115.chip.wasm');
  const module = await WebAssembly.compile(buf);
  const exportsList = WebAssembly.Module
    .exports(module)
    .filter(exp => exp.kind === 'function')
    .map(exp => exp.name);

  console.log('Funções exportadas no WASM:');
  exportsList.forEach(name => console.log(' -', name));

  ['chipInit', 'chipI2CRead', 'chipI2CWrite'].forEach(fn => {
    console.log(`${fn}: ${exportsList.includes(fn) ? '✅ encontrado' : '❌ ausente'}`);
  });
})();