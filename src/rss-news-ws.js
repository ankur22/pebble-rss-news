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
            console.log('Unexpected key received');
            break;
        }
    } else {
        console.log('Message type is null');
    }
  }
);

function getDataForPebble(key, path) {
  var req = new XMLHttpRequest();
  req.open('GET', 'https://rss-news.appspot.com/0/pebble/' + path, true);
  req.onload = function(e) {
    console.log('Received a response for MESSAGE_TYPE ' + key);
    var obj = {};
    if(req.status == 200) {
      var response = JSON.parse(req.responseText);
      if (response.latest !== undefined) {
        console.log('latest lmd: ' + response.latest.lmd);
        obj.GET_LATEST = response.latest.content;
        sendPebbleResponseFromRssNews(obj);
      }
      if (response.top !== undefined) {
        console.log('top lmd: ' + response.top.lmd);
        obj.GET_TOP = response.top.content;
        sendPebbleResponseFromRssNews(obj);
      }
      if (response.categories !== undefined) {
        console.log('categories lmd: ' + response.categories.lmd);
        obj.GET_CATEGORIES = response.categories.content;
        sendPebbleResponseFromRssNews(obj);
      }
    } else {
      console.log('Error: ' + req.status);
      obj.ERROR = req.status.toString();
      sendPebbleResponseFromRssNews(obj);
    }
  };
  req.send(null);
}

function sendPebbleResponseFromRssNews(response) {
  //console.log('Sending to pebble: ' + JSON.stringify(response));
  var transactionId = Pebble.sendAppMessage(response,
    function(e) {
      console.log('Successfully delivered message with transactionId=' + e.data.transactionId);
    },
    function(e) {
      console.log('Unable to deliver message with transactionId=' + e.data.transactionId + ' Error is: ' + e.error.message);
    }
  );
}