function main() {
  const table = document.querySelector(".special-thanks");
  const tbody = document.createElement("tbody");
  table.appendChild(tbody);

  const thanktTo = [
    ["Feyko", "Support and guidance to a noob Unreal Engine modder"],
    ["Robb", "Answering my dumb questions"],
    [
      "Vilsol",
      "Also answering my dumb questions and helping with the documentation system",
    ],
    ["Nog", "Answering the dumbest of my questions"],
    ["Archengius", "Helping with the UE Garbage Collection Issue"],
    ["Deantendo", "Icon/Graphic for FRM"],
    [
      "Andre Aquila",
      "Production Stats code for FRM (Seriously, that would have taken me forever to develop",
    ],
    ["Badger", "For the FRM Companion App"],
    [
      "BLAndrew575",
      "For giving me a crazy world to brutally stress test the getFactory caching function",
    ],
    ["GalaxyVOID", "Contributions to FRM's native web UI"],
    [
      "FeatheredToast",
      "Finding and helping resolve the dumb things I did dumb",
    ],
    [
      "Satisfactory Modding Discord",
      "For motivating me and letting me vent as I go through my day and also develop this mod",
    ],
  ];
  thanktTo.forEach((thanks) => {
    const tr = document.createElement("tr");
    const who = document.createElement("td");
    const why = document.createElement("td");
    who.innerText = thanks[0];
    why.innerText = thanks[1];
    tbody.appendChild(tr);
    tr.appendChild(who);
    tr.appendChild(why);
  });
}
main();
