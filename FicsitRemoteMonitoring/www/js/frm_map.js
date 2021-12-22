window.onload = function () {
	function setupMap() {

		const map = L.map("map", {
			crs: L.CRS.Simple,
			maxZoom: 18,
			minZoom: -10,
			fullscreenControl: true,
		});

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

		map.pm.addControls({
			position: 'topleft',
			drawCircle: false,
		});

		map.on('click', function () {
			centerToPlayer = false;
		});
		
		var realtime1 = createRealtimeLayer('/getPlayer').addTo(map);
		L.control.layers(null, {
			'Markers': realtime1,
		}).addTo(map);

		realtime1.once('update', function () {
			map.fitBounds(realtime1.getBounds(), { maxZoom: 1 });
		});
	};
		
	setTimeout(() => {if (playerLocation) map.setView(playerLocation, -6);}, 1500);

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
			interval: 1 * 1000, // 1 sec

			updateFeature: function (feature, marker) {
				if (!marker) { return; }

				const x = feature.X;
				const y = feature.Y;
				const z = feature.Z;

				var type = feature.type
				var coordinates = feature.coordinates

				// console.info(x, y, L.GeoJSON.coordsToLatLng(coordinates));
				coordinates = worldToMap(coordinates);

				let popup = `<h3>${TypeName[feature.properties.type]}</h3><p>Location: ${x.toFixed(2)}, ${y.toFixed(2)}, ${z.toFixed(2)}</p>`;

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
					popup = popup + `<p>Vel: ${(speed * 0.036).toFixed(2)} km/h *</p>`;
				} else {
					popup = popup + `<p>Vel: ${(vel * 0.036).toFixed(2)} km/h</p>`;
				}

				marker.lastpos = [x, y, z];
				marker.lastupdate = now;                
				marker._popup.setContent(popup);

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

				marker.lastpos = feature.coordinates;
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
	};
	function main() {
			setupMap();
	};

    main();
};