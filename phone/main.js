// const socket = new WebSocket("ws://localhost:4000");

document.getElementById("start-button").addEventListener("click", () => {
  document.getElementById("start-button").remove();

  let gyroscope = new Gyroscope({ frequency: 60 });

  gyroscope.addEventListener("reading", (e) => {
    document.getElementById("debug-info").innerText = `${gyroscope.x}, ${gyroscope.y}, ${gyroscope.z}`;
  });
  gyroscope.start();
});
