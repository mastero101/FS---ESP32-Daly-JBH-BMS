require('dotenv').config();
const express = require('express');
const { Pool } = require('pg');
const bodyParser = require('body-parser');

const app = express();
const port = process.env.PORT || 3000;
const CONTROL_PIN = process.env.CONTROL_PIN || "123456";

// Store pending commands for each ESP32
const pendingCommands = {};

// PostgreSQL Connection
const pool = new Pool({
  user: process.env.DB_USER,
  host: process.env.DB_HOST,
  database: process.env.DB_NAME,
  password: process.env.DB_PASSWORD,
  port: process.env.DB_PORT,
});

// Test & Initialize DB
const initDB = async () => {
  try {
    const res = await pool.query('SELECT NOW()');
    console.log('✅ Conexión a PostgreSQL exitosa:', res.rows[0].now);

    // Create table if not exists
    const createTableQuery = `
      CREATE TABLE IF NOT EXISTS bms_logs (
        id SERIAL PRIMARY KEY,
        hostname VARCHAR(50),
        voltage FLOAT,
        current FLOAT,
        power FLOAT,
        soc FLOAT,
        temp1 FLOAT,
        charge_mos BOOLEAN,
        discharge_mos BOOLEAN,
        connected BOOLEAN,
        rssi INTEGER,
        cells JSONB,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
      );
      CREATE INDEX IF NOT EXISTS idx_bms_logs_created_at ON bms_logs(created_at);
    `;
    await pool.query(createTableQuery);
    console.log('✅ Base de datos inicializada correctamente');
  } catch (err) {
    console.error('❌ Error inicializando PostgreSQL:', err.message);
  }
};

initDB();

// Middleware
app.use(bodyParser.json());
app.use(express.static('public'));

// Endpoint to receive data from ESP32
app.post('/data', async (req, res) => {
  const {
    voltage,
    current,
    soc,
    temp1,
    charge_mos,
    discharge_mos,
    connected,
    hostname,
    rssi,
    cells
  } = req.body;

  const power = voltage * current;
  const cellsJson = JSON.stringify(cells || []);

  console.log(`[${new Date().toISOString()}] Data received from ${hostname} (${voltage}V, ${current}A)`);

  try {
    const query = `
      INSERT INTO bms_logs 
      (hostname, voltage, current, power, soc, temp1, charge_mos, discharge_mos, connected, rssi, cells) 
      VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11) 
      RETURNING *;
    `;
    const values = [hostname, voltage, current, power, soc, temp1, charge_mos, discharge_mos, connected, rssi, cellsJson];
    
    await pool.query(query, values);
    
    // Check if there is a pending command for this ESP32
    const command = pendingCommands[hostname] || null;
    if (command) {
      delete pendingCommands[hostname]; // Clear command after sending
    }

    res.status(201).json({ status: 'success', command });
  } catch (err) {
    console.error('Error inserting data:', err);
    res.status(500).json({ status: 'error', message: err.message });
  }
});

// Endpoint for Dashboard to send commands
app.post('/api/control', (req, res) => {
  const { hostname, type, state, pin } = req.body;

  if (pin !== CONTROL_PIN) {
    return res.status(403).json({ status: 'error', message: 'PIN Incorrecto' });
  }

  pendingCommands[hostname] = { type, state };
  console.log(`[Command Queued] ${type} -> ${state} for ${hostname}`);
  res.json({ status: 'success', message: 'Comando en cola' });
});

// API for Energy (Wh) Today
app.get('/api/energy', async (req, res) => {
  try {
    // Cálculo aproximado de Wh basado en la potencia y el intervalo
    // Asumimos que cada registro representa un intervalo de tiempo (ej: 30s)
    const query = `
      WITH intervals AS (
        SELECT 
          power,
          created_at,
          EXTRACT(EPOCH FROM (created_at - LAG(created_at) OVER (ORDER BY created_at))) / 3600 as hours_diff
        FROM bms_logs
        WHERE created_at >= CURRENT_DATE
      )
      SELECT 
        SUM(CASE WHEN power > 0 THEN power * hours_diff ELSE 0 END) as charged_wh,
        SUM(CASE WHEN power < 0 THEN ABS(power * hours_diff) ELSE 0 END) as discharged_wh
      FROM intervals
      WHERE hours_diff IS NOT NULL AND hours_diff < 0.1; -- Evitar saltos grandes (máx 6 min)
    `;
    
    const result = await pool.query(query);
    res.json(result.rows[0] || { charged_wh: 0, discharged_wh: 0 });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// API for Dashboard
app.get('/api/stats', async (req, res) => {
  try {
    const latestQuery = 'SELECT * FROM bms_logs ORDER BY created_at DESC LIMIT 1';
    const historyQuery = 'SELECT * FROM bms_logs ORDER BY created_at DESC LIMIT 60';
    
    const latest = await pool.query(latestQuery);
    const history = await pool.query(historyQuery);
    
    res.json({
      latest: latest.rows[0],
      history: history.rows
    });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

app.listen(port, () => {
  console.log(`BMS Backend listening at http://localhost:${port}`);
});
