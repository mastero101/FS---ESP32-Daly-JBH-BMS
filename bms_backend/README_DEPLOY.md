# Guía de Despliegue - BMS Gateway

Esta guía detalla los pasos para levantar el sistema utilizando Docker para la base de datos y PM2 para el backend.

## 1. Requisitos Previos
- Docker y Docker Compose instalados.
- Node.js (v16+) y npm instalados.
- PM2 instalado globalmente (`npm install -g pm2`).

## 2. Levantar la Base de Datos (Docker)
Ejecuta el siguiente comando en la carpeta `bms_backend` para levantar el contenedor de PostgreSQL:

```bash
docker-compose up -d
```

Esto creará:
- **Usuario**: `bms_admin`
- **Password**: `bms_secure_pass_2026`
- **Base de Datos**: `bms_database`
- **Persistencia**: Los datos se guardan en el volumen `bms_postgres_data`.

## 3. Configurar el Backend
Asegúrate de que el archivo `.env` tenga las credenciales correctas (ya configuradas por defecto):
```env
PORT=3000
DB_USER=bms_admin
DB_HOST=localhost
DB_NAME=bms_database
DB_PASSWORD=bms_secure_pass_2026
DB_PORT=5432
CONTROL_PIN=123456
```

Instala las dependencias si no lo has hecho:
```bash
npm install
```

## 4. Iniciar el Backend (PM2)
Levanta el servidor Node.js utilizando PM2 para asegurar que se reinicie automáticamente si falla:

```bash
pm2 start index.js --name "bms-backend"
```

### Comandos útiles de PM2:
- Ver logs: `pm2 logs bms-backend`
- Ver estado: `pm2 status`
- Reiniciar: `pm2 restart bms-backend`
- Detener: `pm2 stop bms-backend`

---
**Nota**: El backend creará automáticamente la tabla `bms_logs` la primera vez que reciba datos si esta no existe.
