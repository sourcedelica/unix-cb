//
//  file_store.js
//
//  Text file data store backend for cb_server
//

var path = require('path');
var fs   = require('fs');

var RECS_PATH = process.env.RECS_PATH || path.resolve('../lib/cb/recs');

function userPath(username) {
  return path.join(RECS_PATH, username + '.json');
}

module.exports = {
  getUser: function (username, callback) {
    fs.readFile(userPath(username), {encoding:'UTF-8'}, function (err, data) {
      if (err != null) {
        callback(null, null);
      } else {
        callback(null, JSON.parse(data));
      }
    });
  },

  isExistingUser: function (username, callback) {
    fs.exists(userPath(username), function (exists) {
      callback(null, exists);
    });
  },

  createUser: function (user, callback) {
    fs.writeFile(userPath(user.username), JSON.stringify(user), callback);
  },

  updateUser: function (user, callback) {
    fs.writeFile(userPath(user.username), JSON.stringify(user), callback);
  }

}
