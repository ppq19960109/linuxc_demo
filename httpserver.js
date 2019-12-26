#!/usr/local/bin/node
/*
 * Copyright (c) 2019, Bashi Tech. All rights reserved.
 */

const   request = require("request-promise");
var     settings = require('./settings.js');
var     fs = require('fs');
var     bs58 = require('bs58');
var     crypto = require('crypto');

var     slice_size = 128*1024;
var     cf = settings.load();

var methods = {};

methods.upload = async function upload(filename, slice, sequence){

    // console.log('filename ' + filename + '   ' + sequence);

    var checksum = crypto.createHash('sha256').update(slice).digest();

    var options = {
        method: 'POST',
        uri: 'http://' + cf.host + ':' + cf.port + '/api/upgrade',
        headers:{
            'Content-Type':'application/octet-stream',
            'Content-Length': slice.length,
            'Filename': filename,
            'Sequence': sequence,
            'Checksum': bs58.encode(checksum)
        },

        body: slice,
        json:false
    };

    return await request(options);
}

methods.upgrade = async function(filename) {
    var fbody = fs.readFileSync(filename);
    var count = fbody.length / slice_size;
    var ret;

    for(var i = 0; i < count; i++){
        var slice = Buffer.alloc(slice_size);
        var len = fbody.copy(slice, 0, i*slice_size, (i+1)*slice_size);
        slice = slice.slice(0, len);

        var rsp = await methods.upload(filename, slice, i);

        ret = JSON.parse(rsp);

        if(ret.retcode != 0){
            console.log('upgrade fail : ' + rsp);
            break;
        }
        //console.log('upload slice ' + i);
    }

    return ret;
}

methods.reboot = async function(){
    var body = {
        action : 'reboot',
    }

    var options = {
        method: 'POST',
        uri: 'http://' + cf.host + ':' + cf.port + '/api/reboot',
        headers:{
            'Content-Type':'application/json'
        },

        body: body,
        json:true
    };

    return await request(options);
}

methods.snapshot = async function(){

    var body = {
        action : 'snapshot'
    }

    var options = {
        method: 'POST',
        uri: 'http://' + cf.host + ':' + cf.port + '/api/snapshot',
        headers:{
            'Content-Type':'application/json',
        },

        body: body,
        json:true
    };

    return await request(options);
}

methods.download = async function(snapshotid){

    var body = { 
        action : 'download', 
        snapshot : snapshotid
     };

    var options = {
        method: 'POST',
        uri: 'http://' + cf.host + ':' + cf.port + '/api/download',
        encoding: null,
        headers:{
            'Content-Type':'application/octet-stream',
        },

        body: JSON.stringify(body),
        json:false
    };

    return await request(options);
}

methods.config = async function(ipaddr){

    var body = {
        action : 'config',
        'CFG_SENSOR_AGAIN' : ipaddr,
        'CFG_SENSOR_DGAIN' : ipaddr,
    }

    var options = {
        method: 'POST',
        uri: 'http://' + cf.host + ':' + cf.port + '/api/config',
        headers:{
            'Content-Type':'application/json',
        },

        body: body,
        json:true
    };

    return await request(options);
}


async function selftest(){

/*
    var file_name = 'install/upgrade.bin';
    console.log('upgrade system : ' + file_name);
    var rsp = await methods.upgrade(file_name);
    console.log(rsp);

    console.log('------------');
    console.log('reboot ');
    rsp = await methods.reboot();
    console.log(rsp);

*/
  
    console.log('------------');
    console.log('snapshot ');
    rsp = await methods.snapshot();
    console.log(rsp);
    
    if(rsp.retcode == 0){
        console.log('------------');
        //var snapid = rsp.snapshot;
        var snapid = '1576043538.128182.jpg';
        console.log('download :' + snapid);
        
        rsp = await methods.download(snapid);

        fs.writeFileSync('snapshot-' + snapid, rsp);
    }
  /*
*/
/*
    console.log('------------');
    console.log('config ');
    rsp = await methods.config('12000');
    console.log(rsp);


    console.log('------------');
    console.log('reboot ');
    rsp = await methods.reboot();
    console.log(rsp);

*/
}

module.exports = methods;

selftest();
