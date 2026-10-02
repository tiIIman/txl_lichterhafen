# TXL Lichterhafen

> _TXL Lichterhafen_ macht den früheren Betrieb des Flughafens Tegel
durch eine präzise LED‑Installation wieder erfahrbar.
Bewegungen von Flugzeugen, Abläufen und Betriebsrhythmen
werden als dynamische Lichtspuren visualisiert und in den Stadtraum übersetzt.
So entsteht ein atmosphärisches Erinnerungsbild,
das den Charakter von TXL bewahrt und sichtbar macht.

Dieses Repository enthält den Code, um die Flugdaten zu beschaffen und über den Arduino zu visualisieren.

Folgende Dateien sind enthalten:
- `LiftoffAnimation`: Der Ordner mit der gleichnamigen Datei ist das Ardunio Projekt und der Code, der auf dem Arduino ausgeführt wird.
- `daten_abrufen.py`: Python-Skript zum Abrufen der Flugdaten vom OpenSky Network
- `txl_abfluege_2019.csv`: CSV-Datei mit den Flugdaten des Jahres 2019
- `daten_anreichern.ipynb`: Jupyter Notebook zur Anreicherung der Flugdaten mit zusätzlichen Informationen
- `liftoff_calculation.ipynb`: Jupyter Notebook mit der zugrundeliegenden Berechnung der Startsimulation inkl. eines Graphen
- `txl_abfluege_2019_infos.csv`: CSV-Datei mit zusätzlichen Informationen zu den Flugdaten

Quelle der Flugdaten ist das [OpenSky Network](https://opensky-network.org/).
Um deren API nutzen zu können, muss man sich ein kostenloses Konto anlegen.
Anschließend lässt sich eine `credentials.json` herunterladen,
welche einen persönlichen API-Schlüssel enthält.
Diese Datei wird zum Ausführen der `daten_abrufen.py` benötigt.
Es gibt ein tägliches Limit an API-Anfragen.
Das Abrufen der circa 90.000 Abflüge im Jahr 2019 benötigte drei Tage.
Die API-Dokumentation findet man [hier](https://openskynetwork.github.io/opensky-api/index.html).
