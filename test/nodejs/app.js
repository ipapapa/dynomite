var Db = require('mongodb').Db;
var Connection = require('mongodb').Connection
var Server = require('mongodb').Server
var format = require('util').format;

var mongo = {
        "host": "localhost",
        "port": "27017",
        "db": "mydb"
};

var db = mongo.db;
console.log("Connecting mongo at: " + mongo.host + ":" + mongo.port + ":" + mongo.db);

Db.connect(format("mongodb://%s:%s/%s?w=1", mongo.host, mongo.port, mongo.db), 
   function(err,  db) {
        db.collection('sample', function(err, collection) {
            // Start with a clean collection
            collection.remove({}, function(err, result) {
    
            // Insert a JSON document
            collection.insert({name:'Joe'},{w:1}, function(err,res){    
             collection.findOne({name:'Joe'}, function(err, item) {
              console.log("created at " + new Date(item._id.generationTime) + "\n")   

               // Insert multiple JSON documents with different schema
               for(var i = 0; i < 5; i++) {
                 collection.insert({'a':i}, {w:0});
               }
    
            });
        });
    });
    });
});
