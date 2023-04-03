function main() {
  const themesContainer = document.getElementById("themes-container");

  for (let i = 0; i < themes.themes.length; i++) {
    const themeId = `theme-${i}`;
    const theme = themes.themes[i];
    const themeDiv = document.createElement("div");
    themeDiv.classList.add("theme");
    themeDiv.setAttribute("id", themeId);

    const themePreviewDiv = document.createElement("div");
    themePreviewDiv.classList.add("theme-preview");

    for (let j = 0; j < theme.colors.length; j++) {
      const colorPickerDiv = document.createElement("div");
      colorPickerDiv.classList.add("color-picker");
      colorPickerDiv.style.backgroundColor = theme.colors[j];
      const hints = [
        "Main Color",
        "Second Color",
        "Text Color",
        "Background Color",
        "Table Color",
      ];
      const hint = `${hints[j]}: ${theme.colors[j]}`;
      colorPickerDiv.setAttribute("title", hint);
      themePreviewDiv.appendChild(colorPickerDiv);
    }

    const themeNameH3 = document.createElement("h3");
    themeNameH3.classList.add("theme-name");
    themeNameH3.textContent = theme.name;

    const selectButton = document.createElement("button");
    selectButton.classList.add("select-btn");
    selectButton.textContent = "Select";

    themeDiv.appendChild(themePreviewDiv);
    themeDiv.appendChild(themeNameH3);
    themeDiv.appendChild(selectButton);

    themesContainer.appendChild(themeDiv);
  }

  const selectButtons = document.querySelectorAll(".select-btn");

  selectButtons.forEach((button) => {
    button.addEventListener("click", () => {
      const theme = button.parentNode;
      const themeId = theme.id.split("-").pop();
      localStorage.setItem("selectedTheme", themeId);
      applyTheme();
    });
  });
  const savedThemeId = localStorage.getItem("selectedTheme");

  if (savedThemeId) {
    applyTheme(savedThemeId);
  }
}

main();
