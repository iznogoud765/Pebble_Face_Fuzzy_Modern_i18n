
var mConfig = {};

Pebble.addEventListener("ready", function(e) {
  console.log("fuzzy Modern is ready");
  loadLocalData();
  returnConfigToPebble();
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
      saveLocalData(config);
      returnConfigToPebble();
    }
  }
);

function saveLocalData(config) {

  console.log("saveLocalData() " + JSON.stringify(config));

  localStorage.setItem("BKGD_COLOR_KEY", parseInt(config.KBackgroundColor));  
  localStorage.setItem("ALIGN_KEY", parseInt(config.KAlign)); 
  
  loadLocalData();

}
function loadLocalData() {
  
  mConfig.KBackgroundColor = parseInt(localStorage.getItem("BKGD_COLOR_KEY"));
  if(isNaN(mConfig.KBackgroundColor)) {
    mConfig.KBackgroundColor = 0;
  }

  mConfig.KAlign = parseInt(localStorage.getItem("ALIGN_KEY"));
  if(isNaN(mConfig.KAlign)) {
    mConfig.KAlign = 0;
  }

  //mConfig.configureUrl = "https://sites.google.com/site/pebblefuzzy/index";
  mConfig.configureUrl = "http://iznogoud.olympe.in/pebble/FuzzyModernConfig.html";
  
  console.log("loadLocalData() " + JSON.stringify(mConfig));
}
function returnConfigToPebble() {
  console.log("Configuration window returned: " + JSON.stringify(mConfig));
  Pebble.sendAppMessage({
    "BKGD_COLOR_KEY":parseInt(mConfig.KBackgroundColor), 
    "ALIGN_KEY":parseInt(mConfig.KAlign)
  });    
}
