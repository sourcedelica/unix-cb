//
//  cb_server.js
//
//  telnet server for Skynet Unix-CB
//  Gary Grossman 12/3/2014
//
//  To use on MacOS:
//    # Install Xcode command line tools
//    sudo xcode-select --install
//
//    # Install node.js and redis with Homebrew
//    brew install node
//    brew install redis
//    # Follow instructions displayed for launching Redis.
//
//    cd cb_server
//    npm install
//    export USE_REDIS=T
//    node cb_server.js
//    telnet localhost 8888
//
// Redis is required for the user database to not be lost on shutdown.
// If USE_REDIS=T, Redis is used; otherwise, user records are stored in memory.
//
// pty.js is required for dorkychat. It has to be installed directly from Github
// due to a compile header conflict in the currently released npm version.
//
// Unix-CB must be configured with HOMEREC=0 in this configuration, as each
// user will not have their own real Unix account.
//
//  TODO:
//  - Jumps right into CB after login. Could use a Lobby :)
//

var CB_PATH = require('path').resolve('../cb/CB');
var CB_PORT = process.env.CB_PORT || 8888;

var USE_REDIS = (process.env.USE_REDIS == 'T');

var telnet = require('wez-telnet');
var moment = require('moment');
var bcrypt = require('bcrypt');

var redis = null, redisClient = null;
if (USE_REDIS) {
  redis = require("redis");
  redisClient = redis.createClient(
    process.env.REDIS_PORT || 6379,
    process.env.REDIS_HOST || '127.0.0.1',
    {}
  );
}

function rtrim(s) {
  return s.toString().replace(/\s+$/,'');
}

// Dumb in-memory user database.
var USERS = USE_REDIS ? null : {};

var ONLINE_USERS = {};

function isOnline(user) {
  return ONLINE_USERS[user.username] || false;
}

function setOnline(user) {
  ONLINE_USERS[user.username] = true;
}

function clearOnline(user) {
  ONLINE_USERS[user.username] = false;
}

function echoOff(client)
{
  client.telnetCommand(telnet.WILL, telnet.OPT_ECHO);
}

function echoOn(client)
{
  client.telnetCommand(telnet.WONT, telnet.OPT_ECHO);
}

function redisUserKey(username) {
  return "SkynetUser:"+username;
}

function getUser(username, callback) {
  if (USE_REDIS) {
    redisClient.hgetall(redisUserKey(username), callback);
  } else {
    callback(null, USERS[username]);
  }
}

function createUser(username, password, callback) {
  var user = {
    username: username,
    password: bcrypt.hashSync(password, 10),
    createdAt: moment.utc().format(),
  };
  if (USE_REDIS) {
    redisClient.hmset(redisUserKey(username), user, function (err) {
      callback(err, err != null ? null : user);
    });
  } else {
    USERS[username] = user;
    callback(null, user);
  }
}

function fatal(err) {
  client.write('An error occurred: ' + err.toString());
  client.write('Please try again later.');
  client.end();
}

function createNewUser(client, data) {
  var username = rtrim(data);
  getUser(username, function (err, obj) {
    if (err != null) {
      fatal(err);
    } else if (obj != null) {
      client.write('Sorry, that username is already taken.\r\n\r\n');
      client.write('Enter the username you would like to use: ');
      client.once('data', function (data) { createNewUser(client, data); });
    } else {
      client.write('Enter a password: ');
      echoOff(client);
      function gotPassword(data) {
        client.write('\r\n');
        var password = rtrim(data);
        if (data.length < 5) {
          client.write('Password must be at least 5 characters.\r\n\r\n');
          client.write('Enter a password: ');
          client.once('data', gotPassword);
        } else {
          client.write('Enter it again to be sure: ')
          client.once('data', function (data) {
            client.write('\r\n');
            var password2 = rtrim(data);
            if (password == password2) {
              createUser(username, password, function (err, user) {
                if (err != null) {
                  fatal(err);
                } else {
                  enterCB(client, user);
                }
              });
            } else {
              client.write('Passwords do not match.\r\n\r\n');
              client.write('Enter a password: ');
              client.once('data', gotPassword);
            }
          });
        }
      }
      client.once('data', gotPassword);
    }
  });
}

function newClient(client) {
  client.on('do', function (opt) {
    if (opt != telnet.OPT_ECHO && opt != telnet.OPT_SUPPRESS_GO_AHEAD) {
      client.telnetCommand(telnet.WONT, opt);
    }
  });
  client.write('Welcome to MoHo (Nostlgia Edition)\r\n\r\n');
  client.write('Enter "new" if you are a new user.\r\n')
  login(client, 3);
}

function login(client, retriesLeft) {
  client.write('login: ')
  client.once('data', function (data) {
    var username = rtrim(data);
    if (username == 'new') {
      client.write('Welcome!\r\n\r\n');
      client.write('Enter the username you would like to use: ');
      client.once('data', function (data) { createNewUser(client, data); });
    } else {
      client.write('Password: ');
      echoOff(client);
      client.once('data', function (data) {
        client.write('\r\n');
        var password = rtrim(data);
        getUser(username, function (err, user) {
          if (err != null) {
            fatal(err);
          } else if (user == null || !bcrypt.compareSync(password, user.password)) {
            client.write('Login incorrect.\r\n\r\n');
            if (retriesLeft == 0) {
              client.end();
            } else {
              echoOn(client);
              login(client, retriesLeft-1);
            }
          } else {
            enterCB(client, user);
          }
        });
      });
    }
  });
}

function enterCB(client, user) {
  if (isOnline(user)) {
    client.write('You are already logged in.\r\n');
    client.write('People who talk to themselves never learn anything of value.\r\n');
    client.end();
    return;
  }
  setOnline(user);
  var pty = require('pty.js');
  var term = pty.spawn(CB_PATH, [], {
    name: 'xterm-color',
    cols: 80,
    rows: 30,
    cwd: process.env.HOME,
    env: {PAID:'T', LOGNAME:user.username}
  });
  client.telnetCommand(telnet.WILL, telnet.OPT_SUPPRESS_GO_AHEAD);
  client.telnetCommand(telnet.WILL, telnet.OPT_ECHO);
  term.on('data', function(data) {
    client.write(data);
  });
  term.on('close', function() {
    client.end();
    clearOnline(user);
  });
  client.on('data', function (data) {
    term.write(data);
  });
  client.on('close', function () {
    term.end();
    clearOnline(user);
  });
}

var server = new telnet.Server(newClient);
server.listen(CB_PORT);
