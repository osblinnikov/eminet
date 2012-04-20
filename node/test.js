var EmiNet = require('./eminet');

var es2 = EmiNet.open();
var es = EmiNet.open({
    acceptConnections: true,
    port: 5001
});

es.on('connection', function(socket) {
  console.log("-- Client connected");
  
  socket.on('disconnect', function() {
    console.log("-- Client disconnected");
  });
});

es2.connect('127.0.0.1', 5001, function(err, socket) {
  if (null !== err) {
    return console.log("-- Failed to connect", err);
  }
  
  socket.on('disconnect', function() {
    console.log("-- Disconnected");
  })
  
  console.log("-- Connected", socket);
  socket.close();
});


console.log("Hej", es, es2);

