const map = new L.map("map", {
    crs: L.CRS.Simple,
    maxZoom: 18,
    minZoom: -10,
    fullscreenControl: true,
});

function setupMousePos() {
    L.Control.MousePosition = L.Control.extend({
        options: {
            position: "topleft",
            emptyString: "Unavailable",
            numDigits: 5,
        },

        onAdd: function (map) {
            this._container = L.DomUtil.create(
                "div",
                "leaflet-control-mouseposition"
            );
            L.DomEvent.disableClickPropagation(this._container);
            map.on("mousemove", this._onMouseMove, this);
            this._container.innerHTML = this.options.emptyString;
            return this._container;
        },

        onRemove: function (map) {
            map.off("mousemove", this._onMouseMove);
        },

        _onMouseMove: function (e) {
            const x = L.Util.formatNum(e.latlng.lng, this.options.numDigits);
            const y = L.Util.formatNum(-e.latlng.lat, this.options.numDigits);
            this._container.innerHTML = `x:${x} y:${y}`;
        },
    });

    L.Map.mergeOptions({
        positionControl: false,
    });

    L.Map.addInitHook(function () {
        if (this.options.positionControl) {
            this.positionControl = new L.Control.MousePosition();
            this.addControl(this.positionControl);
        }
    });

    L.control.mousePosition = function (options) {
        return new L.Control.MousePosition(options);
    };
}

function setupMap() {

    const bounds = [
        [375e3, 425301.832031],
        [-375e3, -324698.832031]
    ];

    L.imageOverlay('/img/map.png', bounds).addTo(map);
    map.fitBounds(bounds);
    L.control.mousePosition().addTo(map);

    map.attributionControl.setPrefix('');

    map.setView([0,0] ,-13);

    updateMap();
}

function updateMap() {


    const bounds = [
        [375e3, 425301.832031],
        [-375e3, -324698.832031]
    ];

    map.eachLayer((layer) => {
        layer.remove();
      });

    L.imageOverlay('/img/map.png', bounds).addTo(map);

    createRealtimeLayer("/getPlayer", "Player");
    createRealtimeLayer("/getDrone", "Drone");
    createRealtimeLayer("/getVehicles", "Vehicles");
    createRealtimeLayer("/getTrains", "Trains");
    
}

async function createRealtimeLayer(url, type, container = null) {
    const response = await fetch(url);
    const data = await response.text();
    const getGeo = JSON.parse(data);

    for (var i=0; i<getGeo.length; i++) {

        //This doesn't make sense, but necessary
        var lat = getGeo[i].location.y * -1;
        var lon = getGeo[i].location.x;
        var markerLocation = new L.LatLng(lat, lon);
        var marker = new L.Marker(markerLocation);

        if (type == "Player") {
            var popupText = "Player: " + getGeo[i].PlayerName + "<br>Ping Time: " + getGeo[i].PingTime + " ms";
            marker.setIcon(greenIcon);
        } else if (type == "Drone") {
            var popupText = "Destination: " + getGeo[i].CurrentDestination + "<br>Flying Mode: " + getGeo[i].CurrentFlyingMode;
            marker.setIcon(yellowIcon);
        } else if (type == "Trains") {
            var popupText = "Train Name: " + getGeo[i].TrainName + "<br>Speed: " + getGeo[i].ForwardSpeed + "<br>Derailed: " + getGeo[i].Derailed + "<br>Train Station: " + getGeo[i].TrainStation;
            marker.setIcon(redIcon);
        } else if (type == "Vehicles") {
            var popupText = "Vehicle Type: " + getGeo[i].VehicleType + "<br>AutoPilot: " + getGeo[i].AutoPilot;
            marker.setIcon(greyIcon);
        }

        map.addLayer(marker);
        marker.bindPopup(popupText);

    }
}

function main() {
    setupMousePos();
    setupMap();
}

main();

setInterval(updateMap, 2500);