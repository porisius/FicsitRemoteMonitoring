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
  document.getElementById("map").style.display = "none";
  document.getElementById("PowerTable").style.display = "none";
  document.getElementById("DroneTable").style.display = "none";
  document.getElementById("ProductionTable").style.display = "none";
  document.getElementById("FactoryTable").style.display = "none";
  document.getElementById("About").style.display = "none";
  document.getElementById("Theme").style.display = "none";

  if (show == "map") {
    document.getElementsByTagName("html")[0].style.overflow = "hidden";
    document.getElementsByTagName("body")[0].style.overflow = "hidden";
  } else {
    document.getElementsByTagName("html")[0].style.overflow = "visible";
    document.getElementsByTagName("body")[0].style.overflow = "visible";
  }

  document.getElementById(show).style.display = "";
}

function addButtonsTables() {
  const objects = document.getElementsByClassName("topTable");
  for (var i = 0; i < objects.length; i++) {
    var table = objects[i];
    for (var j = 0; j < table.childNodes.length; j++) {
      var object = table.childNodes[j];

      if (!object.querySelector(".sortContainer")) {
        var sortContainer = document.createElement("div");
        sortContainer.classList = "sortContainer";
        object.appendChild(sortContainer);

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
      }
    }
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
