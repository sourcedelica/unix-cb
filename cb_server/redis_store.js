//
//  redis_store.js
//
//  Redis data store backend for cb_server
//

var redis = require("redis");

var redisClient = redis.createClient(
  process.env.REDIS_PORT || 6379,
  process.env.REDIS_HOST || '127.0.0.1',
  {}
);

redisClient.on("error", function (err) {
  console.log("Redis error: " + err);
});

function redisUserKey(username) {
  return "SkynetUser:"+username;
}

module.exports = {
  getUser: function (username, callback) {
    redisClient.hgetall(redisUserKey(username), callback);
  },

  isExistingUser: function (username, callback) {
    redisClient.exists(redisUserKey(username), callback);
  },

  createUser: function (user, callback) {
    redisClient.hmset(redisUserKey(user.username), user, function (err) {
      callback(err, err != null ? null : user);
    });
  },

  updateUser: function (user, callback) {
    redisClient.hmset(redisUserKey(user.username), user, function (err) {
      callback(err, err != null ? null : user);
    });
  }
}
