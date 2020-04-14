var net = require('net')

var server = net.createServer()
server.on('connection', function(socket) {
    console.log('client connected to server:'+socket.remoteAddress)
    socket.write('connected success')
    
    socket.on('data', function(data) {
        console.log('received data from client is:' + data)
        socket.write('confirmed data ' + data)
    })

    socket.on('end', function() {
        console.log('client connected break off')
    })
})

server.listen(8090, '0.0.0.0', function() {
    console.log('listen client')
})
