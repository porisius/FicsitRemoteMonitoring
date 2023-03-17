const prodstatstimeout = setInterval(catchProdStats, 1000);

catchProdStats().catch((error) => {
  console.log("error! Using test data.");
  console.error(error);
  displayProdStats(test_ProdStats);
});

async function catchProdStats() {
  const response = await fetch("/getProdStats");
  const data = await response.text();
  const getProdStats = JSON.parse(data);
  displayProdStats(getProdStats);
}

var pTable = document.createElement("table");
pTable.style.width = "100%";
pTable.className = "table";
document.getElementById("ProductionTable").appendChild(pTable);

function displayProdStats(getProdStats) {
  var tr = document.createElement("tr");
  tr.className = "topTable";
  pTable.appendChild(tr);

  var lines = [
    "Item Name:",
    "ProdPerMin:",
    "Consumption Percent:",
    "Production Percent:",
    "Current Production:",
    "Maximum Production:",
    "Current Consumed:",
    "Maximum Consumed:",
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

  for (var i in getProdStats) {
    var tr = document.createElement("tr");
    pTable.appendChild(tr);

    var itemName = document.createElement("td");
    itemName.style.alignContent = "left";
    itemName.innerText = getProdStats[i].ItemName;

    var prodPerMin = document.createElement("td");
    prodPerMin.style.alignContent = "center";
    prodPerMin.innerText = getProdStats[i].ProdPerMin;

    var prodPercent = document.createElement("td");
    prodPercent.style.alignContent = "center";
    prodPercent.innerText = getProdStats[i].ProdPercent + "%";

    var consPercent = document.createElement("td");
    consPercent.style.alignContent = "center";
    consPercent.innerText = getProdStats[i].ConsPercent + "%";

    var currentProd = document.createElement("td");
    currentProd.style.alignContent = "center";
    currentProd.innerText = getProdStats[i].CurrentProd.toFixed(2);

    var maxProd = document.createElement("td");
    maxProd.style.alignContent = "center";
    maxProd.innerText = getProdStats[i].MaxProd.toFixed(2);

    var currentConsumed = document.createElement("td");
    currentConsumed.style.alignContent = "center";
    currentConsumed.innerText = getProdStats[i].CurrentConsumed.toFixed(2);

    var maxConsumed = document.createElement("td");
    maxConsumed.style.alignContent = "center";
    maxConsumed.innerText = getProdStats[i].MaxConsumed.toFixed(2);

    var trLoop = [
      itemName,
      prodPerMin,
      prodPercent,
      consPercent,
      currentProd,
      maxProd,
      currentConsumed,
      maxConsumed,
    ];

    for (var i in trLoop) {
      tr.appendChild(trLoop[i]);
    }
  }
}
