const prodstatstimeout = setInterval(catchProdStats, 1000);

catchProdStats().catch(error => {
    console.log('error!');
    console.error(error);
});

async function catchProdStats() {
    const response = await fetch('/getProdStats');
    const data = await response.text();
    const getProdStats = JSON.parse(data);
    displayProdStats(getProdStats);
};

function displayProdStats(getProdStats) {
			
    var i, txt;

    txt = "<table border=1 width=100%>";
    txt += "<tr>";
    txt += "<td><b>Item Name:</b></td><td><b>ProdPerMin:</b></td><td><b>Production Percent:</b></td><td><b>Consumption Percent:</b></td><td><b>Current Production:</b></td><td><b>Maximum Production:</b></td><td><b>Current Consumed:</b></td><td><b>Maximum Consumed:</b></td></tr>";

    for (i in getProdStats)
    {
        txt += '<tr>';
        txt += '<td align="left">' + getProdStats[i].ItemName + '</td>';
        txt += '<td align="center">' + getProdStats[i].ProdPerMin + '</td>';
        txt += '<td align="center">' + getProdStats[i].ProdPercent + '%</td>';
        txt += '<td align="center">' + getProdStats[i].ConsPercent + '%</td>';
        txt += '<td align="center">' + getProdStats[i].CurrentProd.toFixed(2) + '</td>';
        txt += '<td align="center">' + getProdStats[i].MaxProd.toFixed(2) + '</td>';
        txt += '<td align="center">' + getProdStats[i].CurrentConsumed.toFixed(2) + '</td>';
        txt += '<td align="center">' + getProdStats[i].MaxConsumed.toFixed(2) + '</td>';
        txt += '</tr>';
    };
        
    txt += "</table>";
    document.getElementById("ProductionTable").innerHTML = txt;
        
};