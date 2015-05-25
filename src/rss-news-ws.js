Pebble.addEventListener('ready',
  function(e) {
    console.log('JavaScript app ready and running!');
  }
);

Pebble.addEventListener('appmessage',
  function(e) {
    console.log('Received message: ' + JSON.stringify(e.payload));
    getDataForPebble(e.key);
  }
);

function getDataForPebble(key) {
  var req = new XMLHttpRequest();
  req.open('GET', 'https://rss-news.appspot.com/pebble/latest', true);
  req.onload = function(e) {
    if (req.readyState == 4 && req.status == 200) {
      if(req.status == 200) {
        var response = JSON.parse(req.responseText);
        console.log('lmd: ' + response.lmd);
        sendPebbleResponseFromRssNews({key:response});
      } else {
        console.log('Error');
        sendPebbleResponseFromRssNews({key:req.status});
      }
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
      console.log('Unable to deliver message with transactionId=' + e.data.transactionId + ' Error is: ' + e.error.message);
    }
  );
}
