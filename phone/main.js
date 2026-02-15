const socket = new WebSocket("ws://localhost:3000/api/phone_ws/test_lobby");

document.getElementById("start-button").addEventListener("click", () => {
  document.getElementById("start-button").remove();
    document.getElementById("game-display").style.display = "grid";

  let gyroscope = new Gyroscope({ frequency: 60 });

  gyroscope.addEventListener("reading", (e) => {
    document.getElementById("debug-info").innerText = `${gyroscope.x}, ${gyroscope.y}, ${gyroscope.z}`;
    socket.send(JSON.stringify({type: "gyro_update", x: gyroscope.x, y: gyroscope.y, z: gyroscope.z}));
  });
  gyroscope.start();


  // for(let button in ["a", "b", "x", "y"]){
  //   document.getElementById(`button-`).addEventListener("click", () => {});
  // }
});
