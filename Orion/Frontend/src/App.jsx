import { useState } from "react";
import ControlView from "./ControlView";
import Dashboard from "./Dashboard";
import "./App.css";

export default function App() {
  const [view, setView] = useState("control");

  return (
    <div className="app-container">
      <header className="top-bar">
        <button
          className={`nav-btn ${view === "control" ? "active" : ""}`}
          onClick={() => setView("control")}
        >
          Control
        </button>

        <button
          className={`nav-btn ${view === "dashboard" ? "active" : ""}`}
          onClick={() => setView("dashboard")}
        >
          Dashboard
        </button>
      </header>

      <main>{view === "control" ? <ControlView /> : <Dashboard />}</main>
    </div>
  );
}
