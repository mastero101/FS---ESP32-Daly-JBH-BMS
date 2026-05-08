#ifndef WEB_INDEX_H
#define WEB_INDEX_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Daly BMS | BLE Bridge</title>
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;800&display=swap" rel="stylesheet">
  <style>
    :root {
      --bg: #030712;
      --card-bg: #0f172a;
      --border: rgba(255, 255, 255, 0.08);
      --primary: #38bdf8;
      --success: #10b981;
      --warning: #fbbf24;
      --danger: #ef4444;
      --text: #f8fafc;
      --text-dim: #94a3b8;
    }

    * { box-sizing: border-box; margin: 0; padding: 0; }

    body {
      font-family: 'Outfit', sans-serif;
      background-color: var(--bg);
      color: var(--text);
      min-height: 100vh;
      padding: 1.5rem;
    }

    .header {
      text-align: center;
      margin-bottom: 2rem;
      display: flex;
      justify-content: center;
      align-items: center;
      gap: 0.75rem;
      color: var(--primary);
    }

    .header h1 { font-size: 1.5rem; font-weight: 800; letter-spacing: -0.02em; }

    .dashboard-grid {
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      gap: 1.25rem;
      max-width: 1400px;
      margin: 0 auto;
    }

    .card {
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: 1.25rem;
      padding: 1.5rem;
      display: flex;
      flex-direction: column;
      position: relative;
    }

    .card-header {
      display: flex;
      align-items: center;
      gap: 0.75rem;
      margin-bottom: 1.5rem;
      color: var(--primary);
    }

    .card-header svg { width: 20px; height: 20px; stroke-width: 2; fill: none; stroke: currentColor; }
    .card-title { font-size: 0.9rem; font-weight: 600; color: var(--text-dim); }

    .card-value-container { flex: 1; display: flex; flex-direction: column; justify-content: center; }
    
    .card-value { font-size: 2.2rem; font-weight: 800; line-height: 1; margin-bottom: 0.5rem; display: flex; align-items: baseline; }
    .card-unit { font-size: 1.1rem; font-weight: 400; color: var(--text-dim); margin-left: 0.4rem; }

    .card-subtext { font-size: 0.8rem; color: var(--text-dim); margin-top: auto; }

    /* Special Status card */
    .status-text { font-size: 1.5rem; font-weight: 800; color: var(--success); text-transform: uppercase; margin: 1rem 0; }
    .status-text.offline { color: var(--danger); }

    /* Cell Grid Card */
    .grid-cells { grid-column: span 2; }
    .cells-wrapper {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(50px, 1fr));
      gap: 0.5rem;
      margin: 1rem 0;
    }
    .cell-box {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 0.25rem;
    }
    .cell-bar {
      width: 100%;
      height: 40px;
      background: rgba(255,255,255,0.05);
      border-radius: 4px;
      position: relative;
      overflow: hidden;
      display: flex;
      flex-direction: column-reverse;
      border: 1px solid transparent;
    }
    .cell-fill { width: 100%; height: 0%; background: var(--success); transition: height 0.4s; }
    .cell-fill.warning { background: var(--warning); }
    .cell-fill.danger { background: var(--danger); }
    
    .cell-box.max .cell-bar { border-color: var(--warning); }
    .cell-box.min .cell-bar { border-color: var(--danger); }

    .cell-num { font-size: 0.65rem; color: var(--text-dim); }
    .cell-v { font-size: 0.75rem; font-weight: 700; }

    .cell-summary {
      display: flex;
      justify-content: space-between;
      margin-top: 1rem;
      font-size: 0.9rem;
      border-top: 1px solid var(--border);
      padding-top: 1rem;
    }

    /* Temp & MOS Card */
    .mos-row {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-top: 0.75rem;
    }
    .mos-label { font-size: 0.85rem; color: var(--text-dim); }
    .mos-status { font-size: 0.85rem; font-weight: 700; color: var(--success); }
    .mos-status.off { color: var(--danger); }

    /* Switch */
    .switch {
      position: relative;
      display: inline-block;
      width: 40px;
      height: 20px;
    }
    .switch input { opacity: 0; width: 0; height: 0; }
    .slider {
      position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0;
      background-color: #334155; transition: .4s; border-radius: 20px;
    }
    .slider:before {
      position: absolute; content: ""; height: 14px; width: 14px; left: 3px; bottom: 3px;
      background-color: white; transition: .4s; border-radius: 50%;
    }
    input:checked + .slider { background-color: var(--success); }
    input:checked + .slider:before { transform: translateX(20px); }

    /* Bottom Info */
    .footer-actions {
      grid-column: span 4;
      display: flex;
      justify-content: center;
      gap: 1rem;
      margin-top: 1rem;
    }
    .btn {
      padding: 0.6rem 1.2rem;
      border-radius: 0.75rem;
      border: 1px solid var(--border);
      background: var(--card-bg);
      color: var(--text);
      font-family: inherit;
      font-weight: 600;
      cursor: pointer;
      text-decoration: none;
      display: flex;
      align-items: center;
      gap: 0.5rem;
    }
    .btn:hover { background: rgba(255,255,255,0.05); }

    @media (max-width: 1024px) {
      .dashboard-grid { grid-template-columns: repeat(2, 1fr); }
      .grid-cells { grid-column: span 2; }
      .footer-actions { grid-column: span 2; }
    }
    @media (max-width: 640px) {
      .dashboard-grid { grid-template-columns: 1fr; }
      .grid-cells { grid-column: span 1; }
      .footer-actions { grid-column: span 1; }
    }
  </style>
</head>
<body>
  <div class="header">
    <svg viewBox="0 0 24 24" width="24" height="24" stroke="currentColor" stroke-width="2.5" fill="none"><path d="M18 7l4 5-4 5M6 17l-4-5 4-5M14.5 2l-5 20"/></svg>
    <h1>Daly BMS (BLE Bridge)</h1>
  </div>

  <div class="dashboard-grid">
    <!-- Status -->
    <div class="card">
      <div class="card-header">
        <svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="2"/><path d="M16.24 7.76a6 6 0 0 1 0 8.49m-8.48-.01a6 6 0 0 1 0-8.49m11.31-2.82a10 10 0 0 1 0 14.14m-14.14 0a10 10 0 0 1 0-14.14"/></svg>
        <span class="card-title">Estado BMS</span>
      </div>
      <div class="card-value-container">
        <div id="status-text" class="status-text">CONECTADO</div>
      </div>
      <div class="card-subtext">Visto: <span id="sys-last">-</span></div>
    </div>

    <!-- SoC -->
    <div class="card">
      <div class="card-header">
        <svg viewBox="0 0 24 24"><rect x="2" y="7" width="16" height="10" rx="2" ry="2"/><line x1="22" y1="11" x2="22" y2="13"/></svg>
        <span class="card-title">SoC Real (BMS)</span>
      </div>
      <div class="card-value-container">
        <div class="card-value"><span id="soc">0.0</span><span class="card-unit">%</span></div>
      </div>
      <div class="card-subtext">Capacidad BMS 100% Real</div>
    </div>

    <!-- Voltage -->
    <div class="card">
      <div class="card-header">
        <svg viewBox="0 0 24 24"><path d="M13 2L3 14h9l-1 8 10-12h-9l1-8z"/></svg>
        <span class="card-title">Voltaje BMS</span>
      </div>
      <div class="card-value-container">
        <div class="card-value"><span id="voltage">0.00</span><span class="card-unit">V</span></div>
      </div>
      <div class="card-subtext">Monitor de Potencia BMS</div>
    </div>

    <!-- Current -->
    <div class="card">
      <div class="card-header">
        <svg viewBox="0 0 24 24"><path d="M22 12h-4l-3 9L9 3l-3 9H2"/></svg>
        <span class="card-title">Amperaje BMS</span>
      </div>
      <div class="card-value-container">
        <div class="card-value"><span id="current">0.00</span><span class="card-unit">A</span></div>
      </div>
      <div class="card-subtext">Corriente de Carga/Descarga</div>
    </div>

    <!-- Cells Grid (Wide) -->
    <div class="card grid-cells">
      <div class="card-header">
        <svg viewBox="0 0 24 24"><path d="M12 2L2 7l10 5 10-5-10-5zM2 17l10 5 10-5M2 12l10 5 10-5"/></svg>
        <span class="card-title">Estado de Celdas (Pack 8S)</span>
      </div>
      <div id="cells-grid" class="cells-wrapper">
        <!-- Dyn -->
      </div>
      <div class="cell-summary">
        <div>Máx: <span id="cell-max-v" style="color:var(--warning); font-weight:700;">0.000</span>V (#<span id="cell-max-num">-</span>)</div>
        <div>Mín: <span id="cell-min-v" style="color:var(--danger); font-weight:700;">0.000</span>V (#<span id="cell-min-num">-</span>)</div>
        <div>Diff: <span id="cell-diff" style="color:var(--primary); font-weight:700;">0.000</span>V</div>
      </div>
    </div>

    <!-- Temp & MOSFETs -->
    <div class="card">
      <div class="card-header">
        <svg viewBox="0 0 24 24"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/></svg>
        <span class="card-title">Temp BMS</span>
      </div>
      <div class="card-value"><span id="temp">0.0</span><span class="card-unit">°C</span></div>
      
      <div style="margin-top: 1.5rem;">
        <div class="mos-row">
          <span class="mos-label">MOS Carga:</span>
          <div style="display:flex; align-items:center; gap:0.5rem;">
            <span id="mos-c-text" class="mos-status">CONDUCIENDO</span>
            <label class="switch">
              <input type="checkbox" id="sw-charge" onchange="toggleMos('charge', this.checked)">
              <span class="slider"></span>
            </label>
          </div>
        </div>
        <div class="mos-row">
          <span class="mos-label">MOS Descarga:</span>
          <div style="display:flex; align-items:center; gap:0.5rem;">
            <span id="mos-d-text" class="mos-status">CONDUCIENDO</span>
            <label class="switch">
              <input type="checkbox" id="sw-discharge" onchange="toggleMos('discharge', this.checked)">
              <span class="slider"></span>
            </label>
          </div>
        </div>
      </div>
    </div>

    <!-- Power -->
    <div class="card">
      <div class="card-header">
        <svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="10"/><path d="M12 6v6l4 2"/></svg>
        <span class="card-title">Potencia BMS</span>
      </div>
      <div class="card-value-container">
        <div class="card-value"><span id="power">0.0</span><span class="card-unit">W</span></div>
      </div>
      <div class="card-subtext">Cálculo: V x A</div>
    </div>

    <!-- Consumption -->
    <div class="card">
      <div class="card-header">
        <svg viewBox="0 0 24 24"><path d="M12 8v4l3 3m6-3a9 9 0 1 1-9-9 9 9 0 0 1 9 9z"/></svg>
        <span class="card-title">Consumo Hoy (BMS)</span>
      </div>
      <div style="display:flex; flex-direction:column; gap:0.5rem; font-size:0.9rem;">
        <div style="display:flex; justify-content:space-between;"><span style="color:var(--text-dim);">Cargado:</span><span id="stats-in">0.0 Wh</span></div>
        <div style="display:flex; justify-content:space-between;"><span style="color:var(--text-dim);">Descargado:</span><span id="stats-out">0.0 Wh</span></div>
        <div style="display:flex; justify-content:space-between; margin-top:0.5rem; font-weight:700;">
          <span>Neto Hoy:</span><span id="stats-net" style="color:var(--success);">+0.0 Wh</span>
        </div>
      </div>
    </div>

    <div class="footer-actions">
      <button class="btn" onclick="restartSystem()">
        <svg viewBox="0 0 24 24" width="16" height="16" stroke="currentColor"><path d="M23 4v6h-6M1 20v-6h6M3.51 9a9 9 0 0 1 14.85-3.36L23 10M1 14l4.64 4.36A9 9 0 0 0 20.49 15"/></svg>
        Reiniciar ESP32
      </button>
      <a href="/update" class="btn">
        <svg viewBox="0 0 24 24" width="16" height="16" stroke="currentColor"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4M7 10l5 5 5-5M12 15V3"/></svg>
        Actualizar (OTA)
      </a>
      <div class="btn" style="cursor:default; opacity:0.7;">
        IP: <span id="sys-ip">0.0.0.0</span>
      </div>
    </div>
  </div>

  <script>
    function fetchData() {
      fetch('/api/data')
        .then(r => r.json())
        .then(data => {
          document.getElementById('soc').innerText = data.soc.toFixed(1);
          document.getElementById('voltage').innerText = data.voltage.toFixed(2);
          document.getElementById('current').innerText = data.current.toFixed(2);
          document.getElementById('power').innerText = (data.voltage * data.current).toFixed(1);
          document.getElementById('temp').innerText = data.temp1.toFixed(1);
          
          document.getElementById('cell-max-v').innerText = data.cell_max_v.toFixed(3);
          document.getElementById('cell-max-num').innerText = data.cell_max_num;
          document.getElementById('cell-min-v').innerText = data.cell_min_v.toFixed(3);
          document.getElementById('cell-min-num').innerText = data.cell_min_num;
          document.getElementById('cell-diff').innerText = (data.cell_max_v - data.cell_min_v).toFixed(3);

          renderCells(data);

          // MOSFETs
          updateMos('charge', data.charge_mos);
          updateMos('discharge', data.discharge_mos);

          document.getElementById('sys-ip').innerText = data.ip || '-';
          
          const statusTxt = document.getElementById('status-text');
          if (data.connected) {
            statusTxt.innerText = 'CONECTADO';
            statusTxt.className = 'status-text';
          } else {
            statusTxt.innerText = 'DESCONECTADO';
            statusTxt.className = 'status-text offline';
          }

          document.getElementById('sys-last').innerText = new Date().toLocaleTimeString();
          
          // Statistics
          if(data.stats_in !== undefined) {
            document.getElementById('stats-in').innerText = data.stats_in.toFixed(1) + ' Wh';
            document.getElementById('stats-out').innerText = data.stats_out.toFixed(1) + ' Wh';
            let net = data.stats_in - data.stats_out;
            document.getElementById('stats-net').innerText = (net >= 0 ? '+' : '') + net.toFixed(1) + ' Wh';
            document.getElementById('stats-net').style.color = net >= 0 ? 'var(--success)' : 'var(--danger)';
          }
        });
    }

    function renderCells(data) {
      const grid = document.getElementById('cells-grid');
      let count = data.cells ? data.cells.length : 8;
      let html = '';
      for (let i = 1; i <= count; i++) {
        let v = (data.cells && data.cells[i-1]) ? data.cells[i-1] : 
                (i === data.cell_max_num ? data.cell_max_v : 
                (i === data.cell_min_num ? data.cell_min_v : (data.cell_max_v + data.cell_min_v)/2));
        
        let pct = Math.min(100, Math.max(0, ((v - 2.8) / (3.6 - 2.8)) * 100));
        let color = v < 3.0 ? 'danger' : (v > 3.5 ? 'warning' : '');
        
        html += `<div class="cell-box ${i === data.cell_max_num ? 'max' : (i === data.cell_min_num ? 'min' : '')}">
          <div class="cell-bar"><div class="cell-fill ${color}" style="height:${pct}%"></div></div>
          <span class="cell-v">${v.toFixed(3)}</span>
          <span class="cell-num">${i}</span>
        </div>`;
      }
      grid.innerHTML = html;
    }

    function updateMos(type, state) {
      document.getElementById('sw-' + type).checked = state;
      const text = document.getElementById('mos-' + (type === 'charge' ? 'c' : 'd') + '-text');
      text.innerText = state ? 'CONDUCIENDO' : 'BLOQUEADO';
      text.className = 'mos-status' + (state ? '' : ' off');
    }

    function toggleMos(type, state) {
      fetch(`/control?type=${type}&state=${state ? 1 : 0}`)
        .then(r => r.json())
        .then(res => { if(res.status !== 'ok') fetchData(); });
    }

    function restartSystem() {
      if (confirm('¿Reiniciar dispositivo?')) fetch('/restart');
    }

    setInterval(fetchData, 2000);
    fetchData();
  </script>
</body>
</html>
)rawliteral";

#endif
