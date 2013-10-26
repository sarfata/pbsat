function sendTimezoneToWatch() {
  // Get the number of seconds to add to convert localtime to utc
  var offsetMinutes = new Date().getTimezoneOffset() * 60;
  // Send it to the watch
  Pebble.sendAppMessage({ timezoneOffset: offsetMinutes.toString() })
}

function doUpdate() {
  sendTimezoneToWatch();

  /* If geolocation is available, use it! */
  if (navigator.geolocation) {
    console.log("Geolocating ...");
    navigator.geolocation.getCurrentPosition(successfulGeoloc, errorGeoloc);
  }
  else {
    Pebble.sendAppMessage({ 'error': 'No GPS' });
  }
}

PebbleEventListener.addEventListener("ready",
  function(e) {
    console.log("JS Starting...");
  }
);

PebbleEventListener.addEventListener("appmessage", function(e) {
  console.log("Got message from pebble: " + JSON.stringify(e));
  if ("requestUpdate" in e.payload) {
    doUpdate();
  }
});

function successfulGeoloc(position) {
  console.log("Geoloc success: " + JSON.stringify(position));

  var req = new XMLHttpRequest();
  var url = 'http://api.open-notify.org/iss-pass.json?';
  url += 'lat=' + position.coords.latitude;
  url += '&lon=' + position.coords.longitude;
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

