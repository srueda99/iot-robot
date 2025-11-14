import { useEffect, useState } from "react";
import "./Dashboard.css";

export default function Dashboard() {
  const [data, setData] = useState(null);

  const api = "http://3.230.70.191:4040/data";

  useEffect(() => {
    const fetchData = async () => {
      try {
        const res = await fetch(api);
        const json = await res.json();
        setData(json);
      } catch (e) {
        console.error("Error al obtener datos", e);
      }
    };

    fetchData();
    const interval = setInterval(fetchData, 3000);
    return () => clearInterval(interval);
  }, []);

  if (!data) return <p className="loading">Cargando datos...</p>;

  return (
    <div className="dashboard-container">
      {/* TEMPERATURE GAUGE */}
      <div className="temperature-card">
        <h3>Temperatura</h3>

        <div
          className="temp-gauge"
          style={{
            "--temp": data.temp,
          }}
        >
          <div className="temp-inner">
            <span>{data.temp}°C</span>
          </div>
        </div>
      </div>

      {/* HUMIDITY DROP */}
      <div className="humidity-card">
        <h3>Humedad</h3>

        <div className="humidity-drop" style={{ "--humidity": data.hum }}>
          <span>{data.hum}%</span>
        </div>
      </div>

      <div className="location-card">
        <h3>Ubicación</h3>
        <span>{data.lat}° N</span>
        <span>{data.lon}° W</span>
      </div>

      <div className="map-card">
        <iframe
          src={`https://maps.google.com/maps?q=${data.lat},${data.lon}&z=15&output=embed`}
        ></iframe>
      </div>
    </div>
  );
}
