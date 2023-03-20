const dronetimeout = setInterval(catchDrone, 1000);

async function catchDrone() {
  try {
    const response = await fetch("/getDroneStation");
    const data = await response.text();
    const getDrone = JSON.parse(data);
    displayDrone(getDrone);
  } catch {
    displayDrone(test_Drone);
    showAlert("Error while getting drone data! Using Testing Data.");
  }
}

var dTable = document.createElement("table");
dTable.style.width = "100%";
dTable.className = "table";
document.getElementById("DroneTable").appendChild(dTable);

function displayDrone(getDrone) {
  dTable.innerHTML = "";
  var tr = document.createElement("tr");
  tr.className = "topTable";
  dTable.appendChild(tr);

  var lines = [
    "Home Station:",
    "Paired Station:",
    "Drone Status:",
    "Incoming Transfer Rate:",
    "Outgoing Transfer Rate:",
    "Battery Usage Rate:",
  ];

  for (var i = 0; i < lines.length; i++) {
    var td = document.createElement("td");
    var b = document.createElement("b");
    b.innerText = lines[i];
    td.appendChild(b);

    var sortContainer = document.createElement("div");
    sortContainer.classList = "sortContainer";
    td.appendChild(sortContainer);

    var asc = document.createElement("button");
    asc.classList = "sortAsc";
    asc.innerText = "⬆️";
    asc.onclick = sort;

    var desc = document.createElement("button");
    desc.classList = "sortDesc";
    desc.innerText = "⬇️";
    desc.onclick = sort;

    sortContainer.appendChild(asc);
    sortContainer.appendChild(desc);

    tr.appendChild(td);
  }

  for (var i in getDrone) {
    var tr = document.createElement("tr");
    dTable.appendChild(tr);

    var name = document.createElement("td");
    name.innerText = getDrone[i].Name;

    var pairedStation = document.createElement("td");
    pairedStation.innerText = getDrone[i].PairedStation;

    var droneStatus = document.createElement("td");
    droneStatus.innerText = getDrone[i].DroneStatus;

    var avgTotalIncRate = document.createElement("td");
    avgTotalIncRate.innerText = Math.round(getDrone[i].AvgIncRate);

    var avgTotalOutRate = document.createElement("td");
    avgTotalOutRate.innerText = Math.round(getDrone[i].AvgOutRate);

    var estBatteryRate = document.createElement("td");
    estBatteryRate.innerText = Math.round(getDrone[i].EstBatteryRate);

    var trLoop = [
      name,
      pairedStation,
      droneStatus,
      avgTotalIncRate,
      avgTotalOutRate,
      estBatteryRate,
    ];

    for (var i in trLoop) {
      tr.appendChild(trLoop[i]);
    }
  }
}
