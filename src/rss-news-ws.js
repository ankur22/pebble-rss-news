var APP_VERSION = "1.28";
var BASE_URL = "https://rss-news.appspot.com/0/pebble/";
var BASE_URL_V1 = "https://rss-news.appspot.com/1/pebble/";
var BASE_READINGLIST_URL = "rss-news.appspot.com/pebble/";
var USERNAME_KEY = 'USERNAME_KEY';
var CATEGORIES_HEADLINES_KEY = 'CATEGORIES_HEADLINES_KEY';

Pebble.addEventListener('ready',
  function(e) {
    console.log('JavaScript app ready and running!');
    var obj = {};
    obj.HELLO = 'hello';
    sendPebbleResponseFromRssNews(obj);
  }
);

Pebble.addEventListener('showConfiguration',
  function(e) {
    var username = localStorage.getItem(USERNAME_KEY);
    pebbleWatchType = '&pebbleWatchType=' + getWatchType().platform;
    appVersion = '&appVersion=' + APP_VERSION;
    var fullUrl = "https://" + BASE_READINGLIST_URL + username + "?version=0&from=config" + appVersion + pebbleWatchType;
    console.log(fullUrl);
    Pebble.openURL(fullUrl);
});

Pebble.addEventListener('webviewclosed',
  function(e) {
    // Decode and parse config data as JSON
    if (e.response !== undefined && e.response != null && e.response.length > 0) {
        console.log('About to parse response from config: [' + e.response + ']');
        var config_data = JSON.parse(decodeURIComponent(e.response));
        console.log('Config window returned: ', JSON.stringify(config_data));
    }
  }
);

Pebble.addEventListener('appmessage',
  function(e) {
    console.log('Received message: ' + JSON.stringify(e.payload));
    console.log('Key received: ' + e.payload.MESSAGE_TYPE);
    if (e.payload.MESSAGE_TYPE !== null) {
        switch (e.payload.MESSAGE_TYPE) {
          case 4:
              getDataForPebble();
              break;
          default:
            if (e.payload['0'] !== null) {
                console.log('READING_LIST request received: ' + e.payload.MESSAGE_TYPE);
                addToReadingList(e.payload.MESSAGE_TYPE);
            } else {
                console.log('Unexpected key received');
            }
            break;
        }
    } else {
        console.log('Message type is null');
    }
  }
);

function sendFinishedReadingListStuff() {
  var obj = {};
  obj.SAVED_READING_LIST = '';
  sendPebbleResponseFromRssNews(obj);
}

function getWatchType() {
    var current_watch;
    if(Pebble.getActiveWatchInfo) {
      try {
        current_watch = Pebble.getActiveWatchInfo();
      } catch(err) {
        current_watch = {
          platform: "basalt",
        };
      }
    } else {
      current_watch = {
        platform: "aplite",
      };
    }
    return current_watch;
}

function addToReadingList(url) {
  var req = new XMLHttpRequest();
  req.open('POST', BASE_URL + 'readingList?uid=' + encodeURIComponent(url), true);
  req.setRequestHeader('PebbleAccountToken', Pebble.getAccountToken());
  req.setRequestHeader('PebbleWatchType', getWatchType().platform);
  req.setRequestHeader('AppVersion', APP_VERSION);
  req.onload = function(e) {
    if(req.status == 200) {
      console.log('Success: ' + JSON.stringify(req.responseText));
      Pebble.showSimpleNotificationOnPebble("Reading List Updated", "Visit the config page or " + BASE_READINGLIST_URL + JSON.parse(req.responseText).username);
      sendFinishedReadingListStuff();
    } else {
      console.log('Error: ' + req.status + ' ' + JSON.stringify(req.responseText));
      if(req.status == 409) {
          Pebble.showSimpleNotificationOnPebble("Already Saved In Reading List", "Visit the config page or " + BASE_READINGLIST_URL + JSON.parse(req.responseText).username);
          sendFinishedReadingListStuff();
      } else {
          Pebble.showSimpleNotificationOnPebble("Reading List Error", "Visit the config page or " + BASE_READINGLIST_URL + JSON.parse(req.responseText).username);
          sendFinishedReadingListStuff();
      }
    }
  };
  req.send(null);
}

function getDataForPebble() {
  var req = new XMLHttpRequest();

  var fullPath = BASE_URL_V1 + 'all';

  req.open('GET', fullPath, true);
  req.setRequestHeader('PebbleAccountToken', Pebble.getAccountToken());
  req.setRequestHeader('PebbleWatchType', getWatchType().platform);
  req.setRequestHeader('AppVersion', APP_VERSION);
  req.onload = function(e) {
    console.log('Received a response for MESSAGE_TYPE ALL');
    if(req.status == 200) {
      var obj = {};
      var response = JSON.parse(req.responseText);
      if (response.headlines.USA.latest !== undefined) {
        console.log('latest lmd: ' + response.headlines.USA.latest.lmd);
        if (response.headlines.USA.latest.content.length > 0) {
            obj.GET_LATEST = response.headlines.USA.latest.content;
            localStorage.setItem(CATEGORIES_HEADLINES_KEY, response);
        }
      }
      if (response.username !== undefined) {
        console.log('username: ' + response.username);
        if (response.username.length > 0) {
            obj.USERNAME = response.username;
            localStorage.setItem(USERNAME_KEY, response.username);
        }
      }
      obj.ALL = 'all done';
      sendPebbleResponseFromRssNews(obj);
    } else {
      var obj = {};
      var errorMessage = 'Error: ' + req.status;
      console.log(errorMessage);
      obj.ERROR = 'Error';
      sendPebbleResponseFromRssNews(obj);
      sendError(errorMessage);
    }
  };
  req.send(null);
}

function sendError(errorMessage) {
  var req = new XMLHttpRequest();

  var fullPath = BASE_URL + 'error?message=' + encodeURIComponent(errorMessage);
  req.open('POST', fullPath, true);
  req.setRequestHeader('PebbleAccountToken', Pebble.getAccountToken());
  req.setRequestHeader('PebbleWatchType', getWatchType().platform);
  req.setRequestHeader('AppVersion', APP_VERSION);
  req.send(null);
}

function sendPebbleResponseFromRssNews(response) {
  var transactionId = Pebble.sendAppMessage(response,
    function(e) {
      console.log('Successfully delivered message with transactionId=' + e.data.transactionId);
    },
    function(e) {
      console.log('Unable to deliver message with transactionId=' + e.data.transactionId + ' Error is: ' + JSON.stringify(e.data.error));
    }
  );
}