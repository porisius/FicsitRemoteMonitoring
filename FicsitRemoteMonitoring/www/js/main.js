window.onload = function () {
    function setupMousePos() {
        L.Control.MousePosition = L.Control.extend({
            options: {
                position: "bottomleft",
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

    const map = L.map("map", {
        crs: L.CRS.Simple,
        maxZoom: 18,
        minZoom: -10,
        fullscreenControl: true,
    });

    var centerToPlayer = true;
    var playerLocation = null;
    const TypeName = [
        "Default", "Beacon", "Crate", "Hub",
        "Ping", "Player", "Radar Tower", "Resource",
        "Space Elevator", "Starting Pod", "Train", "Train Station",
        "Vehicle", "Vehicle Docking Station", "Drone"
    ];

    function setupMap() {
        const bounds = [
            [-375e3, -324698.832031],
            [375e3, 425301.832031],
        ];

        L.imageOverlay('/img/map.png', bounds).addTo(map);
        map.fitBounds(bounds);
        L.control.mousePosition().addTo(map);
        L.control.measure({
            position: "topleft",
        }).addTo(map);

        /*map.pm.addControls({
            position: 'topleft',
            drawCircle: false,
        });

        map.on('click', function () {
            centerToPlayer = false;
        });

        L.easyButton('<i class="fas fa-crosshairs"></i>', function (btn, map) {
            centerToPlayer = !centerToPlayer;
            if (playerLocation) {
                map.setView(playerLocation);
            }
        }).addTo(map);*/

        // var clusterGroup = L.markerClusterGroup().addTo(map);
        // var subgroup1 = L.featureGroup.subGroup(clusterGroup);
        // var realtime1 = createRealtimeLayer('/api/actors', subgroup1).addTo(map);
        var realtime1 = createRealtimeLayer("/getPlayer").addTo(map);
		var realtime2 = createRealtimeLayer("/getDrone").addTo(map);
		var realtime3 = createRealtimeLayer("/getTrains").addTo(map);
        L.control.layers(null, {
            'Markers': realtime1,
			'Drones': realtime2,
			'Trains': realtime2,
        }).addTo(map);

        realtime1.once('update', function () {
            map.fitBounds(realtime1.getBounds(), { maxZoom: 1 });
        });
		realtime2.once('update', function () {
           map.fitBounds(realtime2.getBounds(), { maxZoom: 1 });
        });
		realtime3.once('update', function () {
           map.fitBounds(realtime3.getBounds(), { maxZoom: 1 });
        });

        setTimeout(() => {if (playerLocation) map.setView(playerLocation, -6);}, 1500);
    }

    function worldToMap(x, y) {
        if (L.Util.isArray(x)) {
            return L.latLng(-x[1], x[0]);
        }
        return L.latLng(-y, x);
    }

    function addMaker(pos) {
        return L.marker(worldToMap(pos));
    }

    function createRealtimeLayer(url, container = null) {
        return L.realtime(url, {
            interval: 2.5 * 1000, // 2.5 sec
            getFeatureId: function (f) {
                return f.properties.index;
            },
            updateFeature: function (feature, marker) {
                if (!marker) { return; }

                const x = feature.geometry.coordinates[0];
                const y = feature.geometry.coordinates[1];
                const z = feature.geometry.coordinates[2];

                var type = feature.geometry && feature.geometry.type
                var coordinates = feature.geometry && feature.geometry.coordinates

                // console.info(x, y, L.GeoJSON.coordsToLatLng(coordinates));
                coordinates = worldToMap(coordinates);

                if (feature.properties.type == 5) {
                    playerLocation = coordinates;
                    if (centerToPlayer) {
                        map.setView(playerLocation);
                    }
                }

                //let popup = `<h3>${TypeName[feature.properties.type]}</h3><p>Location: ${x.toFixed(2)}, ${y.toFixed(2)}, ${z.toFixed(2)}</p>`;

                let vel = 0.0;
                if (feature.properties.vel != null) {
                    for (let i in feature.properties.vel) {
                        vel += Math.pow(feature.properties.vel[i], 2);
                    }
                    vel = Math.sqrt(vel);
                }
                
                const now = performance.now();
                const elapsed = (now - marker.lastupdate) * 0.001;
                const lastpos = marker.lastpos;

                // in cm/s
                const speed = Math.sqrt(Math.pow(lastpos[0] - x, 2) + Math.pow(lastpos[1] - y, 2) + Math.pow(lastpos[2] - z, 2)) / elapsed;
                if (speed > 10 && Math.abs(speed - vel) / speed > 0.3) {
                    //popup = popup + `<p>Vel: ${(speed * 0.036).toFixed(2)} km/h *</p>`;
                } else {
                    //popup = popup + `<p>Vel: ${(vel * 0.036).toFixed(2)} km/h</p>`;
                }

                marker.lastpos = [x, y, z];
                marker.lastupdate = now;                
                //marker._popup.setContent(popup);

                switch (type) {
                    case 'Point':
                        marker.setLatLng(coordinates);
                        break;
                    case 'LineString':
                    case 'MultiLineString':
                        marker.setLatLngs(L.GeoJSON.coordsToLatLngs(coordinates, type === 'LineString' ? 0 : 1));
                        break;
                    case 'Polygon':
                    case 'MultiPolygon':
                        marker.setLatLngs(L.GeoJSON.coordsToLatLngs(coordinates, type === 'Polygon' ? 1 : 2));
                        break;
                    default:
                        return null;
                }
                return marker;
            },
            cache: true,
            container: container,
            onEachFeature(feature, marker) {
                marker.bindPopup(function () {
                    let popup = `<p>Type: ${TypeName[feature.properties.type]}</p>`;
                    if (f.properties.vel != null) {
                        let vel = 0.0;
                        for (let i in feature.properties.vel) {
                            vel += Math.pow(feature.properties.vel[i], 2);
                        }
                        popup = popup + `<p>Vel: ${Math.sqrt(vel)}</p>`;
                    }

                    return popup;
                });

                marker.lastpos = feature.geometry.coordinates;
                marker.lastupdate = performance.now();

                const type = feature.properties.type;
                if (type == 5) {
                    marker.setIcon(greenIcon);
                } else if (type == 1) {
                    marker.setIcon(yellowIcon);
                } else if (type == 3) {
                    marker.setIcon(greyIcon);
                } else if (type == 8) {
                    marker.setIcon(blackIcon);
                } else if (type == 12) {
                    marker.setIcon(violetIcon);
                }
            }
        });
    }

    function main() {
        setupMousePos();
        setupMap();
    }

    main();
};
