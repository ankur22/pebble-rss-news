Pebble.addEventListener('ready',
  function(e) {
    console.log('JavaScript app ready and running!');
    var obj = {};
    obj.HELLO = 'hello';
    sendPebbleResponseFromRssNews(obj);
  }
);

Pebble.addEventListener('appmessage',
  function(e) {
    console.log('Received message: ' + JSON.stringify(e.payload));
    console.log('Key received: ' + e.payload.MESSAGE_TYPE);
    if (e.payload.MESSAGE_TYPE !== null) {
        switch (e.payload.MESSAGE_TYPE) {
          case 4:
              console.log('ALL request received');
              getDataForPebble('ALL', 'all');
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

function addToReadingList(url) {
  var req = new XMLHttpRequest();
  req.open('POST', 'https://rss-news.appspot.com/0/pebble/readingList?url=' + encodeURIComponent(url), true);
  req.setRequestHeader('PebbleAccountToken', Pebble.getAccountToken());
  req.onload = function(e) {
    if(req.status == 200) {
      console.log('Success: ' + JSON.stringify(req.responseText));
      //var obj = {};
      //obj.READING_LIST = req.responseText.username;
      //sendPebbleResponseFromRssNews(obj);
      Pebble.showSimpleNotificationOnPebble("Reading List Updated", "Go to rss-news.appspot.com/0/pebble/readingList/" + JSON.parse(req.responseText).username);
    } else {
      console.log('Error: ' + req.status + ' ' + JSON.stringify(req.responseText));
      //obj.ERROR = req.status.toString();
      //sendPebbleResponseFromRssNews(obj);
      if(req.status == 409) {
          Pebble.showSimpleNotificationOnPebble("Already Saved In Reading List", "Go to rss-news.appspot.com/0/pebble/readingList/" + JSON.parse(req.responseText).username);
      } else {
          Pebble.showSimpleNotificationOnPebble("Reading List Error", "Go to rss-news.appspot.com/0/pebble/readingList/" + JSON.parse(req.responseText).username);
      }
    }
  };
  req.send(null);
}

function getDataForPebble(key, path) {
  var req = new XMLHttpRequest();
  req.open('GET', 'https://rss-news.appspot.com/0/pebble/' + path, true);
  req.onload = function(e) {
    console.log('Received a response for MESSAGE_TYPE ' + key);
    if(req.status == 200) {
      var obj = {};
      var response = JSON.parse(req.responseText);
      if (response.latest !== undefined) {
        console.log('latest lmd: ' + response.latest.lmd);
        obj.GET_LATEST = response.latest.content;
      }
      if (response.top !== undefined) {
        console.log('top lmd: ' + response.top.lmd);
        obj.GET_TOP = response.top.content;
      }
      if (response.categories !== undefined) {
        console.log('categories lmd: ' + response.categories.lmd);
        obj.GET_CATEGORIES = response.categories.content;
      }
      obj.ALL = 'all done';
      sendPebbleResponseFromRssNews(obj);
    } else {
      console.log('Error: ' + req.status);
      obj.ERROR = req.status.toString();
      sendPebbleResponseFromRssNews(obj);
    }
  };
  req.send(null);
}

function sendPebbleResponseFromRssNews(response) {
  var transactionId = Pebble.sendAppMessage(response,
    function(e) {
      console.log('Successfully delivered message with transactionId=' + e.data.transactionId);
    },
    function(e) {
      console.log('Unable to deliver message with transactionId=' + e.data.transactionId + ' Error is: ' + e.data.error);
    }
  );
}