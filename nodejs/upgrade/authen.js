#!/usr/local/bin/node
/*
 * Copyright (c) 2019, Bashi Tech. All rights reserved.
 */

const request = require("request-promise");
var settings = require('./settings.js');
var session = require('./session.js');

var cf = settings.load();

var methods = {};

methods.auth = async function(){

    var options = {
        method: 'POST',
        uri: 'http://' + cf.host + ':' + cf.port + '/api/authen',
        headers:{
            'Content-Type':'application/json'
        },

        body: {
            action: 'authen',
            User: cf.admin,
            Passwd: cf.passwd
        },

        json: true // Automatically stringifies the body to JSON
    };

    var rsp = await request(options);
    if(rsp.retcode == 0){
        session.save(rsp.Token);
    }

    return rsp;
}

methods.changpw = async function(newpasswd){

    var options = {
        method: 'POST',
        uri: 'http://' + cf.host + ':' + cf.port + '/api/authen',
        headers:{
            'Content-Type':'application/json'
        },

        body: {
            action: 'authen',
            User: cf.admin,
            Passwd: cf.passwd,
            newpasswd:newpasswd
        },

        json: true // Automatically stringifies the body to JSON
    };

    var rsp = await request(options);
    if(rsp.retcode == 0){
        session.save(rsp.Token);
    }

    return rsp;

}

methods.activate = async function(){

    var options = {
        method: 'POST',
        uri: 'http://' + cf.host + ':' + cf.port + '/api/authen',
        headers:{
            'Content-Type':'application/json'
        },

        body: {
            action: 'activate',
            User: cf.admin,
            Passwd: cf.passwd
        },

        json: true // Automatically stringifies the body to JSON
    };

    var rsp = await request(options);
    if(rsp.retcode == 0){
        session.save(rsp.Token);
    }

    return rsp;

}

async function selftest(){
    var auth = await methods.auth();
    console.log(auth);

    var cps = await methods.changpw('12345678');
    console.log(cps);
}

module.exports = methods;

selftest();
