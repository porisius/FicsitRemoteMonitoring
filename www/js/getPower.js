const timeout = setInterval(catchPower, 1000);

catchPower().catch(error => {
    console.log('error!');
    console.error(error);
});

async function catchPower() {
    const response = await fetch('/getPower');
    const data = await response.text();
    const getPower = JSON.parse(data);
    displayPower(getPower);
};

function displayPower(getPower) {
    
    var i, txt;

    txt = "<table class='table' width=100%>";
    txt += "<tr>";
    txt += "<td><b>Circuit ID:</b></td><td><b>Power Capacity:</b></td><td><b>Power Production:</b></td><td><b>Power Consumed:</b></td><td><b>Max Consumed:</b></td><td><b>Battery Differential:</b></td><td><b>Battery Percent:</b></td><td><b>Battery Capacity:</b></td><td><b>Time Remaining:</b></td></tr>";

    for (i in getPower)
    {
        txt += '<tr>';
        txt += '<td align="center">' + getPower[i].CircuitID + '</td>';
        txt += '<td align="right">' + getPower[i].PowerCapacity + ' MW</td>';
        txt += '<td align="right">' + getPower[i].PowerProduction + ' MW</td>';
        txt += '<td align="right">' + getPower[i].PowerConsumed + ' MW</td>';
        txt += '<td align="right">' + getPower[i].PowerMaxConsumed + ' MW</td>';
        txt += '<td align="right">' + getPower[i].BatteryDifferential + ' MW</td>';
        txt += '<td align="right">' + getPower[i].BatteryPercent.toFixed(1) + '%</td>';
        txt += '<td align="right">' + getPower[i].BatteryCapacity + ' MW</td>';
        txt += '<td align="right">' + getPower[i].BatteryTimeEmpty + '</td>';
        txt += '</tr>';
    };
        
    txt += "</table>";
    document.getElementById("PowerTable").innerHTML = txt;
};