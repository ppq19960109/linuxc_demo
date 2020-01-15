var net = require("net");

// const PORT = 8090;
// const HOST = '127.0.0.1';

// var clientHandler = function(socket){

//     //客户端发送数据的时候触发data事件
//   socket.on('data', function dataHandler(data) {//data是客户端发送给服务器的数据
//     console.log(socket.remoteAddress, socket.remotePort, 'send', data.toString());
//         //服务器向客户端发送消息
//     socket.write('server received\n');
//   });

//     //当对方的连接断开以后的事件
//   socket.on('close', function(){
//     console.log(socket.remoteAddress, socket.remotePort, 'disconnected');
//   })
// };

// //创建TCP服务器的实例
// //传入的参数是：监听函数clientHandler
// var app = net.createServer(clientHandler);

// app.listen(PORT, HOST);
// console.log('tcp server running on tcp://', HOST, ':', PORT);

var server = net.createServer();
server.on("connection", function(socket) {
    console.log("client connected to server");
    socket.on("data", function(data) {
        console.log("received data from client is:" + data);
        socket.write("confirmed data " + data);
    });
});
server.listen(8090, '127.0.0.1');
