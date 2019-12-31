#!/usr/local/bin/node
/*
 * Copyright (c) 2019, Bashi Tech. All rights reserved.
 */

var fs = require('fs');

var rootpath = __dirname + '/../'

var methods = {};

methods.load = function(){
    var ses = fs.readFileSync('./session.json');

    return JSON.parse(ses);
}

methods.save = function (token){
    var ses = {
        'Token' : token
    };

    fs.writeFileSync('./session.json', JSON.stringify(ses, null, 4));
}

function selftest(){
    var cf = methods.load();

    console.log(JSON.stringify(cf, null, 4));

    cf.Token = '052782c29583d72a731375299ab9298';

    methods.save(cf);

    console.log(JSON.stringify(cf, null, 4));
}

module.exports = methods;

// selftest();