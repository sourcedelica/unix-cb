//
//  dummy_store.js
//
//  In-memory data store backend for cb_server with no persistence
//

var USERS = {};

module.exports = {
  getUser: function (username, callback) {
    callback(null, USERS[username]);
  },

  isExistingUser: function (username, callback) {
    callback(null, USERS[username] != null);
  },

  createUser: function (user, callback) {
    USERS[user.username] = user;
    callback(null);
  },

  updateUser: function (user, callback) {
    USERS[user.username] = user;
    callback(null);
  }
}
