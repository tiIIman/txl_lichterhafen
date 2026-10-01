import os
import time
import csv
from datetime import datetime, timezone, timedelta
from opensky_api import OpenSkyApi, TokenManager

# Konfiguration
AIRPORT_ICAO = "EDDT"  # Kürzel für Berlin-Tegel
START_YEAR = 2019
CSV_FILENAME = "txl_abfluege_2019.csv"

# Zugangsdaten sind in der credentials.json gespeichert
CREDENTIALS_FILE = "credentials.json"
api = OpenSkyApi(token_manager=TokenManager.from_json_file(CREDENTIALS_FILE))

# CSV-Spalten für die Ausgabe
CSV_COLUMNS = [
    "icao24", "callsign", "firstSeen", "lastSeen",
    "estDepartureAirport", "estArrivalAirport",
    "departureAirportCandidatesCount", "arrivalAirportCandidatesCount"
]

def get_last_processed_date():
    """Liest den letzten Zeitstempel aus der CSV, um bei Abbruch nahtlos fortzusetzen."""
    if not os.path.exists(CSV_FILENAME):
        return None

    last_timestamp = None
    with open(CSV_FILENAME, mode="r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            if row.get("firstSeen"):
                last_timestamp = int(row["firstSeen"])

    if last_timestamp:
        return datetime.fromtimestamp(last_timestamp, tz=timezone.utc).date()
    return None

def main():
    # CSV anlegen
    file_exists = os.path.exists(CSV_FILENAME)
    if not file_exists:
        with open(CSV_FILENAME, mode="w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow(CSV_COLUMNS)

    start_date = datetime(START_YEAR, 1, 1, tzinfo=timezone.utc).date()
    end_date = datetime(START_YEAR, 12, 31, tzinfo=timezone.utc).date()

    # Resume-Check
    last_done = get_last_processed_date()
    if last_done and last_done >= start_date:
        current_date = last_done + timedelta(days=1)
        print(f"Download wird ab {current_date} fortgesetzt")
    else:
        current_date = start_date

    total_fetched = 0

    while current_date <= end_date:
        day_start = datetime(current_date.year, current_date.month, current_date.day, 0, 0, 0, tzinfo=timezone.utc)
        day_end = datetime(current_date.year, current_date.month, current_date.day, 23, 59, 59, tzinfo=timezone.utc)

        begin_ts = int(day_start.timestamp())
        end_ts = int(day_end.timestamp())

        print(f"Lade {current_date.strftime('%Y-%m-%d')}...", end=" ", flush=True)

        success = False
        retries = 0

        while not success and retries < 5:
            try:
                departures = api.get_departures_by_airport(AIRPORT_ICAO, begin_ts, end_ts)

                if departures is not None:
                    # In CSV schreiben
                    with open(CSV_FILENAME, mode="a", newline="", encoding="utf-8") as f:
                        writer = csv.writer(f)
                        for f_data in departures:
                            writer.writerow([
                                f_data.icao24,
                                (f_data.callsign or "").strip(),
                                f_data.firstSeen,
                                f_data.lastSeen,
                                f_data.estDepartureAirport,
                                f_data.estArrivalAirport,
                                f_data.departureAirportCandidatesCount,
                                f_data.arrivalAirportCandidatesCount
                            ])

                    count = len(departures)
                    total_fetched += count
                    print(f"OK ({count} Flüge | Gesamt: {total_fetched})")
                else:
                    print("Keine Flüge erfasst.")

                success = True
                # Pause zwischen Anfragen
                time.sleep(1.5)

            except Exception as e:
                retries += 1
                wait_time = retries * 15
                print(f"\nFehler aufgetreten: {e}. Warte {wait_time}s (Versuch {retries}/5)...")
                time.sleep(wait_time)

        if not success:
            print(f"Abbruch am {current_date}")
            break

        current_date += timedelta(days=1)

    print(f"\nFertig! Daten liegen in '{CSV_FILENAME}'.")

if __name__ == "__main__":
    main()