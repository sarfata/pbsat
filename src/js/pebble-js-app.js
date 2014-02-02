/* Convenient function to automatically retry messages. */
Pebble.sendAppMessageWithRetry = function(message, retryCount, successCb, failedCb) {
  var retry = 0;
  var success = function(e) {
    if (typeof successCb == "function") {
      successCb(e);
    }
  };
  var failed = function(e) {
    console.log("Failed sending message: " + JSON.stringify(message) +
      " - Error: " + JSON.stringify(e) + " - Retrying...");
    retry++;
    if (retry < retryCount) {
      Pebble.sendAppMessage(message, success, failed);
    }
    else {
      if (typeof failedCb == "function") {
        failedCb(e);
      }
    }
  };
  Pebble.sendAppMessage(message, success, failed);
};

Pebble.addEventListener("ready",
  function(e) {
    console.log("JS Starting... v1.0.5");
    doUpdate();
  }
);

/* Incoming request */
Pebble.addEventListener("appmessage", function(e) {
  console.log("Got message from pebble: " + JSON.stringify(e));
  if ("requestUpdate" in e.payload) {
    doUpdate();
  }
});

function doUpdate() {
  sendTimezoneToWatch();

  /* If geolocation is available, use it! */
  if (navigator.geolocation) {
    console.log("Geolocating ...");
    navigator.geolocation.getCurrentPosition(successfulGeoloc, errorGeoloc);
  }
  else {
    Pebble.sendAppMessageWithRetry({ 'error': 'No GPS' }, 3);
  }
}

function sendTimezoneToWatch() {
  // Get the number of seconds to add to convert localtime to utc
  var offsetMinutes = new Date().getTimezoneOffset() * 60;
  // Send it to the watch
  Pebble.sendAppMessageWithRetry({ timezoneOffset: offsetMinutes }, 3);
}

function successfulGeoloc(position) {
  console.log("Geoloc success: " + JSON.stringify(position));
  requestSatelliteTracking('ISS', position);
}

function errorGeoloc(msg) {
  console.log("Geoloc failed: " + JSON.stringify(msg));
  Pebble.sendAppMessageWithRetry({ 'error': 'Geoloc failed' }, 3);
}

function requestSatelliteTracking(satellite, position) {
  var req = new XMLHttpRequest();
  var url = 'http://mighty-brushlands-5689.herokuapp.com/';
  url += satellite;
  url += '?latitude=' + position.coords.latitude;
  url += '&longitude=' + position.coords.longitude;
  console.log("Opening: " + url);
  req.open('GET', url, true);
  req.onload = function(e) {
    console.log("onload - readyState: " + req.readyState + " status: " + req.status + " responseType: " + req.responseType);
    if (req.readyState == 4 && req.status == 200) {
      if(req.status == 200) {
        console.log("Got and parsed reply from webservice: " + req.responseText);
        sendTrackingInformation(JSON.parse(req.responseText));
      }
      else {
        console.log("Error ( " + JSON.stringify(req) + ")");
        Pebble.sendAppMessage({ 'error': 'HTTP ' + req.status });
      }
    }
  };
  req.send(null);
}

function sendTrackingInformation(response) {
  // uncomment for demo mode - response = testdata();

  var nextPassage = {
    risetime: response.pass[0].rise.time,
    settime: response.pass[0].set.time
  };
  console.log("Sending: " + JSON.stringify(nextPassage));
  Pebble.sendAppMessageWithRetry(nextPassage, 3, function() {
    // Now send the positions in the sky over time
    var positions = response.pass[0].positions;
    var positionsMessage = {};
    for (var i = 0; i < positions.length; i++) {
      positionsMessage[1000 + 3 * i] = Math.round(positions[i].time);
      positionsMessage[1000 + 3 * i + 1] = Math.round(positions[i].azimuth);
      positionsMessage[1000 + 3 * i + 2] = Math.round(positions[i].altitude);
    }
    console.log("Sending positions: " + JSON.stringify(positionsMessage));
    Pebble.sendAppMessageWithRetry(positionsMessage, 3, function() {
        console.log("Succesfully sent positions to Pebble");
      }, function(error) {
        console.log("Unable to send positions to Pebble: " + JSON.stringify(error));
      }
    );
  });
}


/* Sample data for development */
function testdata() {
  var now = Date.now() / 1000;
  var riseTime = 10 + now;

  return {
    "object": "ISS",
    "observer": {
      "datetime": now,
      "longitude": -122.31236,
      "latitude": 37.5774287
    },
    "pass": [
      {
        "rise": {
            "time": riseTime,
            "azimuth": 171.77353392729623
        },
        "set": {
            "time": riseTime + 476,
            "azimuth": 78.39781970856596
        },
        "transit": {
            "time": riseTime + 238,
            "altitude": 6.985022301420947
        },
        "positions": [
          {
              "time": riseTime,
              "azimuth": 260.77353392729623,
              "altitude": 0.04218089043131164
          },
          {
              "time": riseTime + 60,
              "azimuth": 242.30214592237778,
              "altitude": 30.3545149769612546
          },
          {
              "time": riseTime + 120,
              "azimuth": 220.51563797752684,
              "altitude": 41.629634127462989
          },
          {
              "time": riseTime + 180,
              "azimuth": 190.32722570914922,
              "altitude": 50.350502423281816
          },
          {
              "time": riseTime + 240,
              "azimuth": 160.52337807941521,
              "altitude": 50.984602244786991
          },
          {
              "time": riseTime + 300,
              "azimuth": 135.79987396488805,
              "altitude": 44.288235430600219
          },
          {
              "time": riseTime + 360,
              "azimuth": 110.80591035199153,
              "altitude": 30.533912014453022
          },
          {
              "time": riseTime + 420,
              "azimuth": 92.24688868763702,
              "altitude": 2.2556881160866937
          }
        ]
      }
    ],
    "tles_age": 0
  };
}
