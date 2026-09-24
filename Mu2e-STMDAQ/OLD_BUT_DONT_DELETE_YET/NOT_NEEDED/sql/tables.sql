/* DROP TABLE IF EXISTS slow_control_items ;
CREATE TABLE IF NOT EXISTS slow_control_items (scid SERIAL PRIMARY KEY,  name VARCHAR(64) NOT NULL UNIQUE);

DROP TABLE IF EXISTS slow_control_data ;
CREATE TABLE IF NOT EXISTS slow_control_data (sdid SERIAL PRIMARY KEY,  scid INT REFERENCES slow_control_items (scid), value DOUBLE PRECISION NOT NULL,  time TIMESTAMP NOT NULL);

DROP TABLE IF EXISTS hpge_baseline ;
CREATE TABLE IF NOT EXISTS hpge_baseline (run INT NOT NULL,  subrun INT NOT NULL, mean FLOAT, rms FLOAT, PRIMARY KEY (run, subrun));
*/

DROP TABLE IF EXISTS run_subrun_times ;
CREATE TABLE IF NOT EXISTS run_subrun_times (run INT NOT NULL,  subrun INT NOT NULL, start_time TIMESTAMP, end_time TIMESTAMP, num_int_trigs INT, num_ext_trigs INT, file_name VARCHAR(64), file_format INT, PRIMARY KEY (run, subrun));

DROP TABLE IF EXISTS runinfo ;
CREATE TABLE IF NOT EXISTS runinfo (run SERIAL PRIMARY KEY,  start_time TIMESTAMP, end_time TIMESTAMP, comment VARCHAR(132));
