#!/usr/local/bin/node
/*
 * Copyright (c) 2019, Bashi Tech. All rights reserved.
 */

 
var fs = require('fs');

var methods = {};

methods.load = function(){
    var cf = fs.readFileSync('./settings.json');

    return JSON.parse(cf);
}


function selftest(){
    var cf = methods.load();

    console.log(JSON.stringify(cf, null, 4));
}

module.exports = methods;

// selftest();
