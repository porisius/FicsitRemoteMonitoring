var lastSortDirection = "sortAsc";
var lastSortIndex = 0;
var sortInterval = null;

function defaultColors() {
  var mColor = "#4717f6";
  var sColor = "#4316e6";
  var tColor = "#e7dfdd";
  var tbColor = "#4717f6";
  var bColor = "#121212";

  document.documentElement.style.setProperty("--main-color", mColor);
  document.documentElement.style.setProperty("--secondary-color", sColor);
  document.documentElement.style.setProperty("--text-color", tColor);
  document.documentElement.style.setProperty("--table-color", tbColor);
  document.documentElement.style.setProperty("--background-color", bColor);

  document.getElementById("mainColor").value = mColor;
  document.getElementById("secondColor").value = sColor;
  document.getElementById("textColor").value = tColor;
  document.getElementById("tableColor").value = tbColor;
  document.getElementById("bgColor").value = bColor;

  localStorage.setItem("mainColor", mColor);
  localStorage.setItem("secondColor", sColor);
  localStorage.setItem("textColor", tColor);
  localStorage.setItem("tableColor", tbColor);
  localStorage.setItem("bgColor", bColor);
}

function loadStorage() {
  var mainColor = localStorage.getItem("mainColor");
  var secondColor = localStorage.getItem("secondColor");
  var textColor = localStorage.getItem("textColor");
  var tableColor = localStorage.getItem("tableColor");
  var bgColor = localStorage.getItem("bgColor");

  try {
    if (mainColor != null) {
      document.documentElement.style.setProperty("--main-color", mainColor);
      document.getElementById("mainColor").value = mainColor;
    }
    if (secondColor != null) {
      document.documentElement.style.setProperty(
        "--secondary-color",
        secondColor
      );
      document.getElementById("secondColor").value = secondColor;
    }
    if (textColor != null) {
      document.documentElement.style.setProperty("--text-color", textColor);
      document.getElementById("textColor").value = textColor;
    }
    if (tableColor != null) {
      document.documentElement.style.setProperty("--table-color", tableColor);
      document.getElementById("tableColor").value = tableColor;
    }
    if (bgColor != null) {
      document.documentElement.style.setProperty("--background-color", bgColor);
      document.getElementById("bgColor").value = bgColor;
    }
  } catch {}
}

function updateFirst(event) {
  document.documentElement.style.setProperty(
    "--main-color",
    event.target.value
  );
  localStorage.setItem("mainColor", event.target.value);
}

function updateSecond(event) {
  document.documentElement.style.setProperty(
    "--secondary-color",
    event.target.value
  );
  localStorage.setItem("secondColor", event.target.value);
}

function updateText(event) {
  document.documentElement.style.setProperty(
    "--text-color",
    event.target.value
  );
  localStorage.setItem("textColor", event.target.value);
}

function updateTable(event) {
  document.documentElement.style.setProperty(
    "--table-color",
    event.target.value
  );
  localStorage.setItem("tableColor", event.target.value);
}

function updateBg(event) {
  document.documentElement.style.setProperty(
    "--background-color",
    event.target.value
  );
  localStorage.setItem("bgColor", event.target.value);
}

function menuShow(show) {
  switch (show) {
    case "Home":
      location.href = "/html/home.html";
      break;
    case "Map":
      location.href = "/html/map.html";
      break;
    case "PowerTable":
      location.href = "/html/power.html";
      break;
    case "DroneTable":
      location.href = "/html/drone.html";
      break;
    case "ProductionTable":
      location.href = "/html/prod1.html";
      break;
    case "FactoryTable":
      location.href = "/html/prod2.html";
      break;
    case "About":
      location.href = "/html/about.html";
      break;
    case "Theme":
      location.href = "/html/theme.html";
      break;
    case "Dev":
      location.href = "/html/dev.html";
      break;
    default:
      location.href = "/html/404.html";
      break;
  }
}

function sort(dir) {
  var topTable = dir.srcElement.parentNode.parentNode;
  var tbody = topTable.parentNode.parentNode;
  lastSortIndex = topTable.cellIndex;
  lastSortDirection = dir.srcElement.classList[0];

  if (sortInterval) {
    clearInterval(sortInterval);
  }
  sortInterval = setInterval(() => {
    sortTable(tbody);
  }, 1000);
}

function sortTable(tbody) {
  const rows = Array.from(tbody.querySelectorAll("tr:not([class])"));

  rows.sort((rowA, rowB) => {
    const strA = rowA.cells[lastSortIndex].textContent.toLowerCase();
    const strB = rowB.cells[lastSortIndex].textContent.toLowerCase();

    const numA = parseFloat(rowA.cells[lastSortIndex].textContent);
    const numB = parseFloat(rowB.cells[lastSortIndex].textContent);

    if (numA && numB) {
      switch (lastSortDirection) {
        case "sortAsc":
          return numA - numB;

        case "sortDesc":
          return numB - numA;
      }
    } else if (strA && strB) {
      switch (lastSortDirection) {
        case "sortAsc":
          if (strA < strB) {
            return -1;
          } else if (strA > strB) {
            return 1;
          } else {
            return 0;
          }

        case "sortDesc":
          if (strA < strB) {
            return 1;
          } else if (strA > strB) {
            return -1;
          } else {
            return 0;
          }
      }
    }
  });
  rows.forEach((row) => tbody.appendChild(row));
}

function randomizeTestData() {
  testTable = [test_ProdStats, test_Drone, test_Power, test_Factory];
  testTable.forEach((testData) => {
    for (i in testData) {
      for (j in testData[i]) {
        if (
          typeof testData[i][j] == "number" ||
          typeof testData[i][j] == "float"
        ) {
          testData[i][j] = Math.floor(Math.random() * 10);
        }
      }
    }
  });
  alert("Randomized some data!");
}

function showAlert(text) {
  if (document.querySelector(".alert-container")) return;
  alertContainer = document.createElement("div");
  alertDiv = document.createElement("div");
  alerText = document.createElement("strong");
  alertContainer.classList = "alert-container";
  alertDiv.classList = "alert";
  alerText.classList = "alertText";

  document.body.appendChild(alertContainer);
  alertContainer.appendChild(alertDiv);
  alertDiv.appendChild(alerText);

  alertContainer.style.right = "10px";
  alerText.innerText = text;
  setTimeout(closeAlert, 5000);
}

function closeAlert() {
  document.querySelector(".alert-container").remove();
}

function getMenu() {
  buttons = [
    ["Home", "Home"],
    ["Map", "Map"],
    ["Power", "PowerTable"],
    ["Drone", "DroneTable"],
    ["Overall Production", "ProductionTable"],
    ["Detailed Production", "FactoryTable"],
    ["Dev", "Dev"],
    ["Theme", "Theme"],
    ["About", "About"],
  ];
  menu = document.querySelector(".menu");

  for (i = 0; i < buttons.length; i++) {
    btn = document.createElement("button");
    btn.classList = `button ${buttons[i][1]}`;
    btn.innerText = buttons[i][0];
    btn.onclick = function (button) {
      pressed = buttons.find(
        (element) => element[1] === button.srcElement.classList[1]
      );
      menuShow(pressed[1]);
    };
    menu.appendChild(btn);
  }
}

// Nothing beyond this point only test data.

const test_Drone = [
  {
    ID: 0,
    Name: "Drone Port",
    ClassName: "Build_DroneStation_C_2147259439",
    location: {
      x: -102771.0859375,
      y: -85182.8671875,
      z: 2589.84228515625,
      rotation: 10,
    },
    PairedStation: "Storage Import 2",
    DroneStatus: 3,
    AvgIncRate: 0,
    AvgIncStack: 0,
    AvgOutRate: 73.087936401367188,
    AvgOutStack: 0.47748488187789917,
    AvgRndTrip: "00:02:54",
    AvgTotalIncRate: 0,
    AvgTotalIncStack: 0,
    AvgTotalOutRate: 73.087936401367188,
    AvgTotalOutStack: 0.47748488187789917,
    AvgTripIncAmt: 0,
    EstRndTrip: "00:02:10",
    EstTotalTransRate: 4.1415610313415527,
    EstTransRate: 4.1415610313415527,
    EstLatestTotalIncStack: 0,
    EstLatestTotalOutStack: 0.47748488187789917,
    LatestIncStack: 0,
    LatestOutStack: 0.47748488187789917,
    LatestRndTrip: "00:02:54",
    LatestTripIncAmt: 0,
    LatestTripOutAmt: 212,
    MedianRndTrip: "00:02:54",
    MedianTripIncAmt: 0,
    MedianTripOutAmt: 212,
    EstBatteryRate: 2.7610406875610352,
    features: {
      properties: {
        name: "Drone Port",
        type: "Drone Station",
      },
      geometry: {
        coordinates: {
          X: -102771.0859375,
          Y: -85182.8671875,
          Z: 2589.84228515625,
        },
        type: "Point",
      },
    },
  },
  {
    ID: 1,
    Name: "Drone Port",
    ClassName: "Build_DroneStation_C_2147256765",
    location: {
      x: -105134.5859375,
      y: -85599.7734375,
      z: 2589.84228515625,
      rotation: 10,
    },
    PairedStation: "Battery Import",
    DroneStatus: 4,
    AvgIncRate: 0,
    AvgIncStack: 0,
    AvgOutRate: 33.709449768066406,
    AvgOutStack: 0.16854724287986755,
    AvgRndTrip: "00:02:52",
    AvgTotalIncRate: 0,
    AvgTotalIncStack: 0,
    AvgTotalOutRate: 33.709449768066406,
    AvgTotalOutStack: 0.16854724287986755,
    AvgTripIncAmt: 0,
    EstRndTrip: "00:02:10",
    EstTotalTransRate: 4.1497898101806641,
    EstTransRate: 4.1497898101806641,
    EstLatestTotalIncStack: 0,
    EstLatestTotalOutStack: 0.16854724287986755,
    LatestIncStack: 0,
    LatestOutStack: 0.16854724287986755,
    LatestRndTrip: "00:02:52",
    LatestTripIncAmt: 0,
    LatestTripOutAmt: 97,
    MedianRndTrip: "00:02:52",
    MedianTripIncAmt: 0,
    MedianTripOutAmt: 97,
    EstBatteryRate: 2.766526460647583,
    features: {
      properties: {
        name: "Drone Port",
        type: "Drone Station",
      },
      geometry: {
        coordinates: {
          X: -105134.5859375,
          Y: -85599.7734375,
          Z: 2589.84228515625,
        },
        type: "Point",
      },
    },
  },
];
const test_Factory = [
  {
    Name: "Constructor",
    ClassName: "Build_ConstructorMk1_C",
    location: {
      x: -101321.6640625,
      y: -130824.890625,
      z: -1410.1580810546875,
      rotation: 100,
    },
    Recipe: "Actual Snow",
    RecipeClassName: "",
    production: [
      {
        Name: "Actual Snow",
        ClassName: "Desc_Snow_C",
        Amount: 0,
        CurrentProd: 0,
        MaxProd: 10,
        ProdPercent: 0,
      },
    ],
    ingredients: [
      {
        Name: "FICSMAS Gift",
        ClassName: "Desc_Gift_C",
        Amount: 0,
        CurrentConsumed: 0,
        MaxConsumed: 25,
        ConsPercent: 0,
      },
    ],
    ManuSpeed: 1,
    IsConfigured: true,
    IsProducing: false,
    IsPaused: false,
    CircuitID: 44,
    features: {
      properties: {
        name: "Constructor",
        type: "",
      },
      geometry: {
        coordinates: {
          X: -101321.6640625,
          Y: -130824.890625,
          Z: -1410.1580810546875,
        },
        type: "Point",
      },
    },
  },
];
const test_Power = [
  {
    CircuitID: 58,
    PowerCapacity: 8056.07763671875,
    PowerProduction: 7996.07763671875,
    PowerConsumed: 5760.62841796875,
    PowerMaxConsumed: 9260.1884765625,
    BatteryDifferential: 0,
    BatteryPercent: 100,
    BatteryCapacity: 2500,
    BatteryTimeEmpty: "00:00:00",
    BatteryTimeFull: "00:00:00",
    FuseTriggered: false,
  },
  {
    CircuitID: 9,
    PowerCapacity: 8056.07763671875,
    PowerProduction: 7996.07763671875,
    PowerConsumed: 5760.62841796875,
    PowerMaxConsumed: 9260.1884765625,
    BatteryDifferential: 0,
    BatteryPercent: 100,
    BatteryCapacity: 2500,
    BatteryTimeEmpty: "00:00:00",
    BatteryTimeFull: "00:00:00",
    FuseTriggered: false,
  },
];
const test_ProdStats = [
  {
    Name: "Alclad Aluminum Sheet",
    ClassName: "Desc_AluminumPlate_C",
    ProdPerMin: "P:0.0/min - C: 0.0/min",
    ProdPercent: 0,
    ConsPercent: 0,
    CurrentProd: 0,
    MaxProd: 240,
    CurrentConsumed: 0,
    MaxConsumed: 89.999992370605469,
    Type: "Belt",
  },
  {
    Name: "Alumina Solution",
    ClassName: "Desc_AluminaSolution_C",
    ProdPerMin: "P:0.0/min - C: 0.0/min",
    ProdPercent: 0,
    ConsPercent: 0,
    CurrentProd: 0,
    MaxProd: 720.00006103515625,
    CurrentConsumed: 0,
    MaxConsumed: 660,
    Type: "Pipe",
  },
  {
    Name: "Aluminum Casing",
    ClassName: "Desc_AluminumCasing_C",
    ProdPerMin: "P:0.0/min - C: 0.0/min",
    ProdPercent: 0,
    ConsPercent: 0,
    CurrentProd: 0,
    MaxProd: 240,
    CurrentConsumed: 0,
    MaxConsumed: 180,
    Type: "Belt",
  },
];
