var request = require("request");
var http = require("http");
var querystring=require("querystring");

var options = {
    hostname: "127.0.0.1",
    port: 8080,
    path: "/?name=ppq",
    method: "GET"
};

var body = {
    "name":"ppq"
}

var options_post = {
    hostname: "127.0.0.1",
    port: 8080,
    path: "/",
    method: "POST",
    headers:{
        "Content-Type":"text/plain;charset=UTF-8",
       
    }
};



var req = http.request(options_post, function(res) {
    console.log("STATUS: " + res.statusCode);
    console.log("HEADERS: " + res.headers);
    console.log("HEADERS JSON: " + JSON.stringify(res.headers));
    res.setEncoding("utf8");
    res.on("data", function(chunk) {
        console.log("body:",chunk);
    });
});

// req.write(JSON.stringify(body));
req.end();
