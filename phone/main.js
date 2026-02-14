const socket = new WebSocket("ws://localhost:8080/api/phone_ws");

document.getElementById("start-button").addEventListener("click", () => {
  document.getElementById("start-button").remove();

  let gyroscope = new Gyroscope({ frequency: 60 });

  gyroscope.addEventListener("reading", (e) => {
    const encoder = new TextEncoder();
    document.getElementById("debug-info").innerText = `${gyroscope.x}, ${gyroscope.y}, ${gyroscope.z}`;
    socket.send(encoder.encode(JSON.stringify(JSON.stringify({x: gyroscope.x, y: gyroscope.y, z: gyroscope.z}))));
  });
  gyroscope.start();
});
