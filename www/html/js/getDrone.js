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

  var lines = ["Home Station:", "Current Destination:"];

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

    var homeStation = document.createElement("td");
    homeStation.innerText = getDrone[i].HomeStation;

    var currentDestination = document.createElement("td");
    currentDestination.innerText = getDrone[i].CurrentDestination;

    var trLoop = [homeStation, currentDestination];

    for (var i in trLoop) {
      tr.appendChild(trLoop[i]);
    }
  }
}
