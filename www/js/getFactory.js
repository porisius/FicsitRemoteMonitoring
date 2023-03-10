const factorytimeout = setInterval(catchProdStats, 1000);

catchFactory().catch((error) => {
  console.log("error! Using test data.");
  console.error(error);
  displayFactory(test_Factory);
});

async function catchFactory() {
  const response = await fetch("/getFactory");
  const data = await response.text();
  const getFactory = JSON.parse(data);
  displayFactory(getFactory);
}

function displayFactory(getFactory) {
  var i, p, n, txt, prod, need, eff;

  txt = "<table class='table' width=100%>";
  txt += "<tr class='topTable'>";
  txt +=
    "<td><b>Building Type:</b></td><td><b>Recipe:</b></td><td><b>Efficiently:</b></td><td><b>Recipe / Current Production:</b></td><td><b>Ingredients / Current Consumption:</b></td><td><b>Manufacturing Speed:</b></td><td><b>Producing:</b></td><td><b>Paused:</b></td></tr>";

  for (i in getFactory) {
    prod = "";
    need = "";

    for (p in getFactory[i].production) {
      prod +=
        "Output: " +
        getFactory[i].production[p].Name +
        " / " +
        getFactory[i].production[p].CurrentProd.toFixed(2) +
        "<br>";
    }

    for (n in getFactory[i].ingredients) {
      need +=
        "Output: " +
        getFactory[i].ingredients[n].Name +
        " / " +
        getFactory[i].ingredients[n].CurrentConsumed.toFixed(2) +
        "<br>";
    }

    eff = getFactory[i].production[p].ProdPercent * 1;

    txt += "<tr>";
    txt += '<td align="left">' + getFactory[i].building + "</td>";
    txt += '<td align="center">' + getFactory[i].Recipe + "</td>";
    txt += '<td align="center">' + eff.toFixed(2) + "%</td>";
    txt += '<td align="left">' + prod + "</td>";
    txt += '<td align="left">' + need + "</td>";
    txt += '<td align="center">' + getFactory[i].ManuSpeed * 100 + "%</td>";
    txt += '<td align="center">' + getFactory[i].IsProducing + "</td>";
    txt += '<td align="center">' + getFactory[i].IsPaused + "</td>";
    txt += "</tr>";
  }

  txt += "</table>";
  document.getElementById("FactoryTable").innerHTML = txt;
}
