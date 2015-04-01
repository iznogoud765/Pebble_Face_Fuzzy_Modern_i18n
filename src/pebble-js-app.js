
var mConfig = {};

Pebble.addEventListener("ready", function(e) {
  console.log("fuzzy Modern is ready");
  loadLocalData();
});

Pebble.addEventListener("showConfiguration", function(e) {
	console.log("open URL");
  Pebble.openURL(mConfig.configureUrl);
	
});

Pebble.addEventListener("webviewclosed",
  function(e) {
	  console.log("webviewclosed reponse=" + e.response.length + ", " + e.response);
    if (e.response) {
      var config = JSON.parse(decodeURIComponent(e.response));
      returnConfigToPebble(config);
    }
  }
);

function loadLocalData() {
  
  mConfig.KBackgroundColor = 0;
  mConfig.KAlign = 0;

  //mConfig.configureUrl = "https://sites.google.com/site/pebblefuzzy/index";
  mConfig.configureUrl = "http://iznogoud.olympe.in/pebble/FuzzyModernConfig.html";
  
  //console.log("loadLocalData() " + JSON.stringify(mConfig));
}

function returnConfigToPebble(config) {
  console.log("Configuration window returned: " + JSON.stringify(config));
  Pebble.sendAppMessage({
    "BKGD_COLOR_KEY":parseInt(config.KBackgroundColor), 
    "ALIGN_KEY":parseInt(config.KAlign)
  });    
}
