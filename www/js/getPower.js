const timeout = setInterval(catchPower, 1000);

catchPower().catch((error) => {
  console.log("error! Using test data.");
  console.error(error);
});

async function catchPower() {
  try {
    const response = await fetch("/getPower");
    const data = await response.text();
    const getPower = JSON.parse(data);
    displayPower(getPower);
  } catch {
    displayPower(test_Power);
  }
}

var powerTable = document.createElement("table");
powerTable.style.width = "100%";
powerTable.className = "table";
document.getElementById("PowerTable").appendChild(powerTable);

function displayPower(getPower) {
  powerTable.innerHTML = "";
  var tr = document.createElement("tr");
  tr.className = "topTable";
  powerTable.append(tr);

  var lines = [
    "Reset Circuit:",
    "Circuit ID:",
    "Power Capacity:",
    "Power Production:",
    "Power Consumed:",
    "Max Consumed:",
    "Battery Differential:",
    "Battery Percent:",
    "Battery Capacity:",
    "Time Remaining:",
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

  for (var i in getPower) {
    var tr = document.createElement("tr");
    powerTable.appendChild(tr);

    var resetCircuit = document.createElement("td");
    var resetCircuitBtn = document.createElement("button");
    resetCircuitBtn.addEventListener("click", function () {
      fetch(
        "/setCircuit?circuit=" +
          this.parentNode.parentNode.childNodes[1].innerText +
          "&action=reset"
      );
    });
    resetCircuitBtn.className = "resetCircuit";
    resetCircuitBtn.innerText = "Reset";
    resetCircuit.style.alignContent = "center";
    resetCircuit.appendChild(resetCircuitBtn);

    var circuitID = document.createElement("td");
    circuitID.innerText = getPower[i].CircuitID;
    circuitID.style.alignContent = "center";

    var powerCapacity = document.createElement("td");
    powerCapacity.innerText = Math.round(getPower[i].PowerCapacity) + "MW";
    powerCapacity.style.alignContent = "right";

    var powerProduction = document.createElement("td");
    powerProduction.innerText = Math.round(getPower[i].PowerProduction) + "MW";
    powerProduction.style.alignContent = "right";

    var powerConsumed = document.createElement("td");
    powerConsumed.innerText = Math.round(getPower[i].PowerConsumed) + "MW";
    powerConsumed.style.alignContent = "right";

    var powerMaxConsumed = document.createElement("td");
    powerMaxConsumed.innerText =
      Math.round(getPower[i].PowerMaxConsumed) + "MW";
    powerMaxConsumed.style.alignContent = "right";

    var batteryDifferential = document.createElement("td");
    batteryDifferential.innerText =
      Math.round(getPower[i].BatteryDifferential) + "MW";
    batteryDifferential.style.alignContent = "right";

    var batteryPercent = document.createElement("td");
    batteryPercent.innerText = getPower[i].BatteryPercent.toFixed(1) + "%";
    batteryPercent.style.alignContent = "right";

    var batteryCapacity = document.createElement("td");
    batteryCapacity.innerText = Math.round(getPower[i].BatteryCapacity) + "MW";
    batteryCapacity.style.alignContent = "right";

    var batteryTimeEmpty = document.createElement("td");
    batteryTimeEmpty.innerText = getPower[i].BatteryTimeEmpty;
    batteryTimeEmpty.style.alignContent = "right";

    var trLoop = [
      resetCircuit,
      circuitID,
      powerCapacity,
      powerProduction,
      powerConsumed,
      powerMaxConsumed,
      batteryDifferential,
      batteryPercent,
      batteryCapacity,
      batteryTimeEmpty,
    ];

    for (var i in trLoop) {
      tr.appendChild(trLoop[i]);
    }
  }
}
