function jsonStuff() {
    links = [
        "getBelts",
        "getDoggo",
        "getDrone",
        "getDroneStation",
        "getDropPod",
        "getExtractor",
        "getHUBTerminal",
        "getModList",
        "getPipes",
        "getPlayer",
        "getPower",
        "getPowerSlug",
        "getProdStats",
        "getRadarTower",
        "getResourceNode",
        "getResourceSink",
        "getSchematics",
        "getSinkList",
        "getSpaceElevator",
        "getStorageInv",
        "getSwitches",
        "getTrainStation",
        "getTrains",
        "getTruckStation",
        "getWorldInv"
    ];
    
    linksSelect = document.getElementById("links")
    for (var i = 0; i < links.length; i++) {
        option = document.createElement("option")
        option.value = links[i]
        option.innerText = links[i]
        linksSelect.appendChild(option)
    }
}

function magic() {
    fetch("/"+linksSelect.selectedOptions[0].innerText)
  .then((response) => response.json())
  .then((data) => {
    code = document.querySelector(".codeThing")
    code.innerHTML = toString(data)
    Prism.highlightAll()
  });
}