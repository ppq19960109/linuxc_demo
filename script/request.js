var request = require("request");
var settings = require('./settings.js');

var ipaddr=settings.load();

var body = {
    "name":"ppq"
}

var options_post = {
    url: "http://"+ipaddr.host+":"+ipaddr.port+"/",  
    method: "POST",
    headers:{
        "Content-Type":"text/plain;charset=UTF-8",
       
    },
    json: true,
    body: JSON.stringify(body)
};



request(options_post, function(error, response, body) {
    console.info('error: ' + error );
    console.log("statusCode: " + response.statusCode );
    console.log("response JSON: " + JSON.stringify(response));
    console.info('body: ' + body );
    console.info('body JSON: ' + JSON.stringify(body) );
});

