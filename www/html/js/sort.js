function sort(dir) {
  var topTable = dir.srcElement.parentNode.parentNode;
  var tbody = document.querySelector("tbody");
  lastSortIndex = topTable.cellIndex;
  lastSortDirection = dir.srcElement.classList[0];

  switch (lastSortDirection) {
    case "sortAsc":
      asc = false;
      break;
    case "sortDesc":
      asc = true;
      break;
  }

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
    const cellA =
      rowA.cells[lastSortIndex].textContent ||
      rowA.cells[lastSortIndex].innerText;
    const cellB =
      rowB.cells[lastSortIndex].textContent ||
      rowA.cells[lastSortIndex].innerText;

    const v1 = asc ? cellA : cellB;
    const v2 = asc ? cellB : cellA;

    return v1 !== "" && v2 !== "" && !isNaN(v1) && !isNaN(v2)
      ? v1 - v2
      : v1.toString().localeCompare(v2);
  });
  rows.forEach((row) => tbody.appendChild(row));
}
