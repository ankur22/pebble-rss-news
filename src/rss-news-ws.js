Pebble.addEventListener('ready',
  function(e) {
    console.log('JavaScript app ready and running!');
    sendPebbleResponseFromRssNews({'HELLO':'Hello'});
  }
);

Pebble.addEventListener('appmessage',
  function(e) {
    console.log('Received message: ' + JSON.stringify(e.payload));
    console.log('Key received: ' + e.payload.MESSAGE_TYPE);
    if (e.payload.MESSAGE_TYPE !== null) {
        switch (e.payload.MESSAGE_TYPE) {
          case 1:         // GET_LATEST
              console.log('GET_LATEST request received');
              getDataForPebble('GET_LATEST', 'latest');
              break;
          case 2:         // GET_TOP
              console.log('GET_TOP request received');
              getDataForPebble('GET_TOP', 'top');
              break;
          case 3:         // GET_CATEGORIES
              console.log('GET_CATEGORIES request received');
              getDataForPebble('GET_CATEGORIES', 'categories');
              break;
          case 4:         // ALL
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
    obj = {};
    if(req.status == 200) {
      var response = JSON.parse(req.responseText);
      console.log('lmd: ' + response.lmd);
      obj[key] = response;
      sendPebbleResponseFromRssNews(obj);
    } else {
      console.log('Error: ' + req.status);
      obj['ERROR'] = req.status.toString();
      sendPebbleResponseFromRssNews(obj);
    }
  };
  req.send(null);
}

function sendPebbleResponseFromRssNews(response) {
  console.log('Sending to pebble: ' + JSON.stringify(response));
  var transactionId = Pebble.sendAppMessage(response,
    function(e) {
      console.log('Successfully delivered message with transactionId=' + e.data.transactionId);
    },
    function(e) {
      console.log('Unable to deliver message with transactionId=' + e.data.transactionId + ' Error is: ' + e.error.message);
    }
  );
}
