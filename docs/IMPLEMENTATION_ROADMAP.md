# 10derCAE – verbindlicher Implementierungsplan

Ein Meilenstein gilt erst als abgeschlossen, wenn Implementierung, automatisierte
Tests, Fehlerbehandlung, Speichern/Laden, Undo/Redo und die kritischen manuellen
Interaktionen geprüft sind.

## M1 – Interaktiver 2D-Skizzierer

- [erledigt] Skizzierebenen XY/XZ/YZ, Raster, Zoom und Pan
- [erledigt] Linie, Rechteck, Kreis und Drei-Punkt-Bogen per Maus
- [erledigt] Auswahl, Mehrfachauswahl, Löschen und Abbrechen mit Escape
- [erledigt] Raster- und Endpunktfang mit automatischer Koinzidenz
- [erledigt] horizontal, vertikal, parallel, lotrecht, koinzident und fixiert
- [erledigt] Längen-, Punktabstands-, Radius- und Winkelmaße
- [erledigt] sichtbare Constraint- und Maß-Glyphen
- [erledigt] Bogenradius, Konzentrizität und äußere Tangentialität
- [erledigt] Symmetrie zweier gleichartiger Geometrien an einer Konstruktionslinie
- [erledigt] innere Tangentialität der Trägerkreise, Typprüfung und JSON-Roundtrip
- [offen] Kontaktpunktprüfung innerhalb endlicher Bogenabschnitte
- Freiheitsgrad-, Unter-/Voll-/Überbestimmungsanzeige
- [erledigt] Serialisierung und Solver-Regressionstests
- [erledigt] geschlossene Profilprüfung und Extrusionsübergabe für Linien/Kreis

## M2 – Parametrische Bauteilfeatures

- Extrusion: Neu, Vereinigung, Differenz und Schnittmenge
- Drehung, Bohrung, Sweep und Loft
- gezielte Kantenwahl für Fase und Rundung
- Wandung, Spiegelung und lineare/zirkulare Muster
- Editieren, Unterdrücken, Löschen und robuste Fehlerzustände
- semantische Topologiereferenzen und vollständige Neuberechnung

## M3 – Baugruppen

- separate Bauteildokumente und Komponenteninstanzen
- Transformationsmanipulator, Fixieren und Freiheitsgrade
- Mate/Flush, konzentrisch, Winkel, Tangente und Einfügen
- Gelenke: starr, Dreh-, Schub-, zylindrisch und planar
- Kollisionsprüfung, Stückliste und fehlende Referenzen

## M4 – Zeichnungen

- normgerechte Blätter und Schriftfelder
- Erst-, Projektions-, Schnitt- und Detailansichten
- assoziative Maße, Mittellinien, Symbole und Beschriftungen
- Aktualisierung nach Modelländerungen, PDF/DXF-Export

## M5 – FEM

- Materialdaten, Randbedingungen, Lasten und Kontakte
- Netgen-Netzadapter, Qualitätsprüfung und Visualisierung
- linear-statischer Solveradapter, Spannungen und Verschiebungen
- Einheiten-, Plausibilitäts- und Referenztests

## M6 – Dynamik und Systemmodell

- Chrono-Adapter für starre Mehrkörpersysteme
- Antriebe, Kräfte, Federn, Kontakte und Ergebnisplots
- FMI/SUNDIALS-basierte Blocksimulation
- Kopplung von Mechanik und Systemmodell

## M7 – Produktreife

- Performance-, Absturz-, DPI- und Screenshotregressionen
- große Baugruppen, Wiederherstellung und Migration älterer Dokumente
- Barrierefreiheit, vollständige DE/EN-Texte und Hilfesystem
- signiertes, reproduzierbares Windows-Paket

