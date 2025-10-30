import React, { useState } from "react";
import "./App.css";

export default function App() {
  const [speed, setSpeed] = useState(50);
  const api = "http://backend:4040/status";
  const handleControl = (command) => {
    console.log(`Comando: ${command}, Velocidad: ${speed}%`);
    const response = fetch(api, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        "x-api-key": "AK90YTFGHJ007WQ",
      },
      body: JSON.stringify({ cmd: command.toUpperCase(), speedness: speed }),
    });
    if (!response.ok) {
      console.log("Fallo al enviar instrucciones");
      console.log(response.statusText);
    }
  };

  const handleSpeedChange = (e) => {
    setSpeed(Number(e.target.value));
  };

  return (
    <div className="control-container">
      <div className="joystick">
        <button onClick={() => handleControl("forward")} className="btn up">
          ↑
        </button>
        <div className="middle-row">
          <button onClick={() => handleControl("left")} className="btn left">
            ←
          </button>
          <button onClick={() => handleControl("stop")} className="btn stop">
            ⏹
          </button>
          <button onClick={() => handleControl("right")} className="btn right">
            →
          </button>
        </div>
        <button onClick={() => handleControl("backward")} className="btn down">
          ↓
        </button>
      </div>

      <div className="speed-control">
        <div
          className="speed-ring"
          style={{
            "--speed": speed,
          }}
        >
          <input
            type="range"
            min="0"
            max="100"
            value={speed}
            onChange={handleSpeedChange}
            className="speed-slider"
          />
        </div>
        <div className="speed-value">{speed}%</div>
      </div>
    </div>
  );
}
