const express = require('express');
const cors = require('cors');

const app = express();
const port = process.env.PORT || '4040';
const key = 'AK90YTFGHJ007WQ';
var instruction = "STOP";
var speed = 0;
var latitud = 0;
var longitud = 0;
var temperatura = 0;
var humedad = 0;

app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

app.listen(port, () => {
  console.log(`Orion API corriendo en http://localhost:${port}`);
});

app.get('/', (req, res) => {
  res.send('Orion API etá corriendo.');
});

// Endpoint para recibir y actualizar instrucciones
app.get('/status', (req, res) => {
  res.status(200).send({ command: instruction, speedness: speed });
});

app.post('/status', (req, res) => {
  const apiKey = req.headers['x-api-key'];
  if (!apiKey) {
    return res.status(401).send('Se requiere clave API.');
  }
  if (apiKey !== key) {
    return res.status(403).send('Clave API inválida.');
  }
  const { cmd, speedness } = req.body;
  instruction = cmd;
  speed = speedness;
  console.log(`Instrucción actualizada: ${instruction}`);
  console.log(`Velocidad actualizada: ${speed}%`);
  res.status(200).send('Instrucción y velocidad actualizadas.');
});

// Endpoint para recibir los datos d elos sensores
app.get('/data', (req, res) => {
  res.status(200).send({ lat: latitud, lon: longitud, temp: temperatura, hum: humedad });
});

app.post('/data', (req, res) => {
  const { lat, lon, temp, hum } = req.body;
  latitud = lat;
  longitud = lon;
  temperatura = temp;
  humedad = hum;
  console.log(`Datos recibidos - Latitud: ${lat}, Longitud: ${lon}, Temperatura: ${temp}°C, Humedad: ${hum}%`);
  res.status(200).send('Datos recibidos correctamente.');
});