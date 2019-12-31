 
var fs = require('fs')

// var methods = {}
class methods {}

methods.load = function(){
    var cf = fs.readFileSync('./settings.json')

    return JSON.parse(cf)
}

module.exports = methods

