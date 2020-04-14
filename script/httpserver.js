var http = require("http");

http.createServer(function(req, res) {
    console.log(req.url);

    res.writeHead(200, { "Content-Type": "text/html" });
    res.write("<h1>Node.js</h1>");
    res.end("<p>Hello World</p>");
}).listen(8090);

console.log("HTTP server is listening at port 8090.");
