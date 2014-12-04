//
//  cb_server.js
//
//  telnet server for Skynet Unix-CB
//  Gary Grossman 12/3/2014
//
//  To use:
//    npm install pty
//    node cb_server.js
//    telnet localhost 8888
//
//  TODO:
//  - Right now, user records are stored in-memory and lost on shutdown
//  - Should be more configurable: read TCP port from ENV, etc.
//  - Jumps right into CB after login. Could use a Lobby :)
//

var CB_PATH = require('path').resolve('../cb/CB');
var CB_PORT = 8888;

var telnet = require('./telnet.js');

function rtrim(s) {
  return s.toString().replace(/\s+$/,'');
}

// Telnet stuff
var WILL        = 251; // I will use option
var WONT        = 252; // I won't use option
var TELOPT_ECHO = 1;   // echo
var TELOPT_SGA  = 3;   // suppress go ahead

// Dumb in-memory user database.
var USERS = {};

function echoOff(client)
{
  client.telnetCommand(WILL, TELOPT_ECHO);
}

function echoOn(client)
{
  client.telnetCommand(WONT, TELOPT_ECHO);
}

function createNewUser(client, data) {
  var username = rtrim(data);
  if (USERS[username]) {
    client.write('Sorry, that username is already taken.\r\n\r\n');
    client.write('Enter the username you would like to use: ');      
    client.once('data', function (data) { createNewUser(client, data); });
  } else {
    client.write('Enter a password: ');
    echoOff(client);
    client.once('data', function (data) {
      client.write('\r\n');
      var password = rtrim(data);
      var user = {username:username, password:password, online:false};
      USERS[username] = user;
      enterCB(client, user);
    });
  }
}

function newClient(client) {
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
        var user = USERS[username];
        if (user == null || user.password != password) {
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
    }
  });
}

function enterCB(client, user) {
  if (user.online) {
    client.write('You are already logged in.\r\n');
    client.write('People who talk to themselves never learn anything of value.\r\n');
    client.end();
    return;
  }
  user.online = true;
  var pty = require('pty');
  var term = pty.spawn(CB_PATH, [], {
    name: 'xterm-color',
    cols: 80,
    rows: 30,
    cwd: process.env.HOME,
    env: {PAID:'T', LOGNAME:user.username}
  });
  client.telnetCommand(WILL, TELOPT_SGA);
  client.telnetCommand(WILL, TELOPT_ECHO);
  term.on('data', function(data) {
    client.write(data);
  });
  term.on('close', function() {
    client.end();
    user.online = false;
  });
  client.on('data', function (data) {
    term.write(data);
  });
  client.on('close', function () {
    term.end();
    user.online = false;
  });
}

var server = new telnet.Server(newClient);
server.listen(CB_PORT);
