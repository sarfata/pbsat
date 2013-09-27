function sendTimezoneToWatch() {
  // Get the number of seconds to add to convert localtime to utc
  var offsetMinutes = new Date().getTimezoneOffset() * 60;
  // Send it to the watch
  Pebble.sendAppMessage({ timezoneOffset: offsetMinutes.toString() })
}

PebbleEventListener.addEventListener("ready",
  function(e) {
    var isReady = e.ready;
    if (isReady) {
      sendTimezoneToWatch();

      /* If geolocation is available, use it! */
      if (navigator.geolocation) {
        console.log("Geolocating ...");
        navigator.geolocation.getCurrentPosition(successfulGeoloc, errorGeoloc);
      }
      else {
        console.log("Geolocation not available - Reverting to hardcoded Palo Alto, CA");
        var hardcodedPosition = {
          coordinates: {
            latitude: 37.440278,
            longitude: -122.158611,
            accuracy: 100
          },
          timestamp: Date.now()
        };
        successfulGeoloc(hardcodedPosition);
      }
    }
  }
);

function successfulGeoloc(position) {
  console.log("Geoloc success: " + position);

  var req = new XMLHttpRequest();
  var url = 'http://api.open-notify.org/iss-pass.json?';
  url += 'lat=' + position.coordinates.latitude;
  url += '&lon=' + position.coordinates.longitude;
  console.log("Opening: " + url);
  req.open('GET', url, true);
  req.onload = function(e) {
    console.log("onload - readyState: " + req.readyState + " status: " + req.status + " responseType: " + req.responseType);
    if (req.readyState == 4 && req.status == 200) {
      if(req.status == 200) {
        console.log("Got and parsed reply from webservice: " + req.responseText);
        var response = JSON.parse(req.responseText);

        var nextPassage = {
          risetime: response.response[0].risetime.toString(),
          duration: response.response[0].duration.toString()
        };
        console.log("Sending: " + JSON.stringify(nextPassage));
        Pebble.sendAppMessage(nextPassage);

/*
        var utc = new Date(now.getTime() + now.getTimezoneOffset() * 60000);

        var nextPassageTimeout = response.response[0].risetime - Date.now() / 1000;

        console.log("next passage in " + nextPassageTimeout + " seconds");


        var timeout = setTimeout(function() {

        }, );
*/
      }
      else {
        console.log("Error");
      }
    }
  }
  req.send(null);
}

function errorGeoloc(msg) {
  console.log("Geoloc failed: " + msg);
}

