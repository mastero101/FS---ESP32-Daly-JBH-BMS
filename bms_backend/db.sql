-- Create the database
-- CREATE DATABASE bms_db;

-- Create the table for BMS logs
CREATE TABLE bms_logs (
    id SERIAL PRIMARY KEY,
    hostname VARCHAR(50),
    voltage FLOAT,
    current FLOAT,
    power FLOAT, -- New column for V * I
    soc FLOAT,
    temp1 FLOAT,
    charge_mos BOOLEAN,
    discharge_mos BOOLEAN,
    connected BOOLEAN,
    rssi INTEGER,
    cells JSONB, -- New column for individual cell voltages
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Index for faster queries on hostname and date
CREATE INDEX idx_bms_hostname ON bms_logs(hostname);
CREATE INDEX idx_bms_created_at ON bms_logs(created_at);
