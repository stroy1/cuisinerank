function fetchRestaurants(coordinates) {
  var KEY = "";
  
  var response; 
  console.log(coordinates.latitude);
  console.log(coordinates.longitude);
  var req = new XMLHttpRequest();
  
  req.open('HEAD', "https://maps.googleapis.com/maps/api/place/textsearch/json?" /
           + "query=restaurants&location="+coordinates.latitude +"," + coordinates.longitude /
           + "&radius=5&key=" + KEY, true);
  
  req.onload = function(e) {
    console.log("inside onload statement");
    //console.log("object: " + req.status);
    //console.log("response: " + req.responseText);
    //if (req.readyState == 4) {
      if(req.status == 200) {
        console.log("inside if statement");
        console.log(req.responseText);
        response = JSON.parse(req.responseText);
        //console.log(response);

        /*var breakfast, lunch, dinner;
        //if (response && response.results && response.results.length > 0) {
          var cuisineResult = response.results;
          breakfast = cuisineResult.Breakfast[0].Breakfast.replace("\n", " ");
          lunch = cuisineResult.Lunch[0].Lunch.replace("\n", " ");
          dinner = cuisineResult.Dinner[0].Dinner.replace("\n", " ");
          console.log(breakfast);
          console.log(lunch);
          console.log(dinner);
          Pebble.sendAppMessage( { 0: breakfast, 1: lunch, 2: dinner});*/

      } else {
       console.log("Error");
        console.log(req);
      }
    //}
  };
    
  req.send();
    //req.send(null);
  }
//}

/*function HTTPGET(url) {
    var req = new XMLHttpRequest();
    req.open("GET", url, false);
    req.send(null);
    return req.responseText;
}
var getWeather = function() {
    var lhs1 = "usd";
    var rhs1 = "jpy";
    var url1 = "rate-exchange.appspot.com/currency?from=" + lhs1 + "&to=" + rhs1
    console.log(url1);
    var response1 = HTTPGET(url1);
    var json1 = JSON.parse(response1);
*/




var locationOptions = {
  enableHighAccuracy: true,
  timeout: 60000,
  maximumAge: 0
};

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    0:"Loc Unavailable",
    1:"N/A"
  });
}


Pebble.addEventListener("ready",
                        function(e) {
                          console.log("ready location!" + e.ready);
                          navigator.geolocation.getCurrentPosition(function(position) {
                            console.log("lat ready pos" + (position.coords.latitude));
                            Pebble.sendAppMessage({ 0: String(position.coords.latitude), 1: String(position.coords.longitude)});
                          },locationError, locationOptions);
                           //do_something(position.coords.latitude, position.coords.longitude);
                        });


Pebble.addEventListener("appmessage",
                        function(e) {
                          console.log("attempt location!");
                          navigator.geolocation.getCurrentPosition(
                            function(position) {
                              console.log("lat pos: " + (position.coords.latitude));
                              
                              fetchRestaurants(position.coords);
                              /*Pebble.sendAppMessage({ 0: String(position.coords.latitude), 1: String(position.coords.longitude)},function(f){
                                console.log("message sent js");},
                                  function(f){
                                    console.log("message FAILED");
                                  }
                          );*/
                            
                        },locationError, locationOptions);
                           //do_something(position.coords.latitude, position.coords.longitude);
            });


Pebble.addEventListener("webviewclosed",
                        function(e) {
                          console.log("webview closed");
                          console.log(e.type);
                          console.log(e.response);
                        });




