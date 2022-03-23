const dronetimeout = setInterval(catchDrone, 1000);

catchDrone().catch(error => {
    console.log('error!');
    console.error(error);
});

async function catchDrone() {
    const response = await fetch('/getDroneStation');
    const data = await response.text();
    const getDrone = JSON.parse(data);
    displayDrone(getDrone);
};

function displayDrone(getDrone) {
			
    var i, txt;

    txt = "<table border=1 width=100%>";
    txt += "<tr>";
    txt += "<td><b>Home Station:</b></td><td><b>Paired Station:</b></td><td><b>Drone Status:</b></td><td><b>Incoming Transfer Rate:</b></td><td><b>Outgoing Transfer Rate:</b></td><td><b>Battery Usage Rate:</b></td></tr>";

    for (i in getDrone)
    {
        txt += '<tr>';
        txt += '<td>' + getDrone[i].HomeStation + '</td>';
        txt += '<td>' + getDrone[i].PairedStation + '</td>';
        txt += '<td>' + getDrone[i].DroneStatus + '</td>';
        txt += '<td>' + getDrone[i].AvgTotalIncRate + '</td>';
        txt += '<td>' + getDrone[i].AvgTotalOutRate + '</td>';
        txt += '<td>' + getDrone[i].EstBatteryRate + '</td>';
        txt += '</tr>';
    };
        
    txt += "</table>";
    document.getElementById("DroneTable").innerHTML = txt;
        
};