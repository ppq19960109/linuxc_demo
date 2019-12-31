#!/usr/local/bin/node
/*
 * Copyright (c) 2019, Bashi Tech. All rights reserved.
 */

const request = require("request-promise");
var settings = require('./settings.js');
var session = require('./session.js');
var fs = require('fs');
var bs58 = require('bs58');
var crypto = require('crypto');


var slice_size = 128*1024;

var cf = settings.load();
var ses = session.load();

var methods = {};

methods.upload = async function upload(filename, slice, sequence){

    console.log('upload filename ' + filename + ' seq:' + sequence);

    var checksum = crypto.createHash('sha256').update(slice).digest();

    var options = {
        method: 'POST',
        uri: 'http://' + cf.host + ':' + cf.port + '/api/upgrade',
        headers:{
            'Content-Type':'application/application/octet-stream',
            'Content-Length': slice.length,
            'Filename': filename,
            'Sequence': sequence,
            'Checksum': bs58.encode(checksum),
            'Token':ses.Token
        },

        body: slice,
        json:false
    };

    return await request(options);
}

methods.upgrade = async function(filename) {
    var fbody = fs.readFileSync(filename);
    var count = fbody.length / slice_size;


    console.log('upgrade file : ' + filename + '  size : ' + fbody.length);

    for(var i = 0; i < count; i++){
        var slice = Buffer.alloc(slice_size);
        var len = fbody.copy(slice, 0, i*slice_size, (i+1)*slice_size);
        slice = slice.slice(0, len);


        console.log('upload slice offset : ' + i*slice_size + '  size : ' + len);

        var rsp = await methods.upload(filename, slice, i);
        var ret = JSON.parse(rsp);

        if(ret.retcode != 0){
            console.log('upgrade fail : ' + rsp);
            break;
        }
        //console.log('upload slice ' + i);
    }
}

methods.reboot = async function(){
    var body = {
        action : 'restart',
    }

    var options = {
        method: 'POST',
        uri: 'http://' + cf.host + ':' + cf.port + '/api/restart',
        headers:{
            'Content-Type':'application/json',
            'Token':ses.Token
        },

        body: body,
        json:true
    };

    return await request(options);
}

async function selftest(){

    var file_name = 'upgrade.bin';

    var rsp = await methods.upgrade(file_name);
    console.log(rsp);

    rsp = await methods.reboot();
    console.log(rsp);
}

module.exports = methods;

selftest();
