document.getElementById("start-button").addEventListener("click", () => {
  const socket = new WebSocket(`ws://foxmoss.com:3000/api/phone_ws/${encodeURIComponent(document.getElementById("lobby").value)}`);
  document.getElementById("lobby-handler").remove();
  document.getElementById("game-display").style.display = "grid";

  let gyroscope = new Gyroscope({ frequency: 60 });

  gyroscope.addEventListener("reading", (e) => {
    document.getElementById("debug-info").innerText = `${gyroscope.x}, ${gyroscope.y}, ${gyroscope.z}`;
    socket.send(JSON.stringify({type: "gyro_update", x: gyroscope.x, y: gyroscope.y, z: gyroscope.z}));
  });
  gyroscope.start();


  let buttons =  ["a", "b", "x", "y"];
  for(let button in buttons){
    document.getElementById(`button-${buttons[button]}`).addEventListener("mousedown", (e) => {
      socket.send(JSON.stringify({type: "button_down", button: button}));
    });
    document.getElementById(`button-${buttons[button]}`).addEventListener("mouseup", (e) => {
      socket.send(JSON.stringify({type: "button_up", button: button}));
    });
  }
});
