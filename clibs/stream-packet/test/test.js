var spack = require('../build/Release/stream_packet')
var index = spack.get();
var o = spack.recv(index, '002400000000000aeyJhIjoxLCJiIjoyfQ==', false);
console.log(o)
var o = spack.send(index, 10, '{"a":"\0","b":"\0"}')
console.log(o)
var o = spack.recv(index, o.data, false);
console.log(o)
spack.put(index);
