import { useState } from "react";
import "./App.css";

export default function ControlView() {
  const [speed, setSpeed] = useState(50);
  const [error, setError] = useState(null);

  const api = "http://3.230.70.191:4040/status";

  const handleControl = async (command) => {
    try {
      const response = await fetch(api, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
          "x-api-key": "AK90YTFGHJ007WQ",
        },
        body: JSON.stringify({ cmd: command, speedness: speed }),
      });

      if (!response.ok) throw new Error("Error HTTP");
      setError(null);
    } catch (err) {
      setError("Error al enviar datos");
    }
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
        <div className="speed-ring" style={{ "--speed": speed }}>
          <input
            type="range"
            min="0"
            max="100"
            value={speed}
            onChange={(e) => setSpeed(Number(e.target.value))}
            className="speed-slider"
          />
        </div>
        <div className="speed-value">{speed}%</div>
      </div>
    </div>
  );
}
