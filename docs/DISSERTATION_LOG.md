# 10derCAE – Entwicklungs- und Dissertationsjournal

## Versuch: robuste Viewer-Komposition und Symmetrie (2026-10-15)

Der OCCT-Renderer lieferte korrekte Framebuffer-Dumps, wurde im nativen Qt-Fenster
jedoch nur als kleine Teilfläche vor schwarzem Hintergrund dargestellt. Ursache war
die konkurrierende Darstellung eines nativen OpenGL-Kindfensters und der normalen
Qt-Widget-Hierarchie. Der Viewport rendert deshalb nun mit `V3d_View::ToPixMap` in
ein geräteauflösendes Bild, das Qt im regulären Paint-Ereignis zusammensetzt. Eine
unverfälschte Bildschirmaufnahme bei 1440 x 920 bestätigt die vollflächige Anzeige
von Körper, Kanten, Achsenkreuz und Navigationswürfel.

Die Symmetriebedingung referenziert zwei gleichartige Skizzenelemente und eine
nicht entartete Linienachse. Punkte werden orthogonal auf die unendliche Achse
projiziert und mit dem doppelten Projektionsvektor gespiegelt. Linien spiegeln beide
Endpunkte, Kreise Mittelpunkt und Radius, Drei-Punkt-Bögen alle Definitionspunkte.
Ist eine Konstruktionslinie unter drei selektierten Elementen vorhanden, erkennt die
Oberfläche sie unabhängig von der internen Selektionsreihenfolge als Achse. Referenzen
auf die dritte Entität werden im JSON-Format gespeichert und beim Entfernen der Achse
zusammen mit der Bedingung gelöscht.

Dieses Dokument protokolliert fortlaufend wissenschaftlich und technisch relevante
Erkenntnisse des Projekts. Es wird ausschließlich während der Entwicklung ergänzt
und ist kein Bestandteil der Programmlaufzeit.

## Entwicklungsstand 2026-10-16: innere Tangentialität

Der neue serialisierte Constraint-Typ `internalTangent` verwendet den Abstand
`abs(radiusA - radiusB)` der Trägerkreismittelpunkte. Der Solver verschiebt eine
nicht fixierte Kurve, erhält Radius und Bogenwinkel und berücksichtigt auch eine
fixierte zweite Kurve. Bei anfangs identischen Mittelpunkten wird die positive
X-Richtung als reproduzierbare Verschiebungsrichtung gewählt. Gleiche Radien und
Linien werden beim Anlegen abgewiesen. Geprüft werden Kreis/Kreis, Kreis/Bogen,
Bogen/Kreis und Bogen/Bogen sowie JSON-Roundtrip und die Canvas-Befehlskette.

Grenze: Tangentialität betrifft zunächst die vollständigen Trägerkreise; die Lage
des Kontaktpunkts innerhalb beider endlicher Bogenabschnitte ist noch offen.
Der Symmetrie-Interaktionstest fixiert nun seine Referenzlinie, damit die interne
Qt-Selektionsreihenfolge keine unterschiedlichen gültigen Spiegelresultate erzeugt.

## Projektziel

Kontaktpunktpruefung (2026-10-16): Nach den iterativen Projektionsschritten wird der
geometrische Beruehrpunkt fuer aeussere/innere Kurventangentialitaet sowie Linie/Bogen
bestimmt. Der Winkelbereich des Drei-Punkt-Bogens wird ueber seinen Zwischenpunkt
ausgewaehlt; dadurch sind beide Umlaufrichtungen und Boegen ueber 180 Grad erfasst.
Endpunkte gelten innerhalb der Laengentoleranz als enthalten. Kontakte ausserhalb
des Bogens erscheinen mit ihrer Constraint-ID in der bestehenden Fehlerdiagnose.
Eine automatische Drehung oder Aenderung des Bogenwinkels erfolgt nicht.
Tests umfassen beide Referenzreihenfolgen, innere/aeussere Kontakte, ungueltige
Trägerkreiskontakte, grosse/kleine Boegen, JSON-Roundtrip und das Canvas-Statussignal.
Geraden behalten die Semantik einer unendlichen Traegergeraden. Die frueher in
diesem Journal dokumentierte Beschraenkung auf Traegerkreise ist damit behoben.

Entwicklung 2026-10-16: Ein dauerhaft sichtbarer Start-Button im Ribbon-Kopf
aktiviert die vorhandene Startseite (Tastatur: Alt+Home). Ein Interaktionstest prüft
den Workspacewechsel und wiederholtes Betätigen. Der Skizzensolver liefert zusätzlich
die IDs der in der letzten Iteration nicht erfüllten Bedingungen. Das ist eine
Konvergenzdiagnose, noch kein mathematischer Nachweis eines minimalen Konfliktsatzes.
Beim Entfernen einer Fixierbedingung wird der Fixierungszustand neu abgeleitet;
ein Regressionstest prüft Konflikterkennung und anschließende Lösbarkeit.

10derCAE untersucht, wie sich eine integrierte, native CAD/CAE-Umgebung mit
Open-Source-Komponenten aufbauen lässt. Der Funktionsumfang soll parametrische
Bauteilmodellierung, Baugruppen, Zeichnungen, FEM, Mehrkörpersimulation und
Systemsimulation in einer konsistenten Bedienoberfläche zusammenführen.

Die Bedienabläufe orientieren sich an etablierten CAD-Konventionen und öffentlich
dokumentierten Arbeitsweisen von Autodesk Inventor. Markenassets, proprietärer Code
und nicht freigegebene Autodesk-Ressourcen werden nicht übernommen.

## Forschungsfragen

1. Wie robust lässt sich ein historienbasierter parametrischer Modellierer auf OCCT
   aufbauen?
2. Wie können geometrische und dimensionale Skizzenabhängigkeiten nachvollziehbar
   gelöst und ihre verbleibenden Freiheitsgrade dargestellt werden?
3. Welche stabilen Referenzen werden benötigt, damit Topologieänderungen nach einer
   Modellneuberechnung abhängige Features möglichst selten beschädigen?
4. Wie können CAD, FEM, Mehrkörper- und Systemsimulation ein gemeinsames Dokument-
   und Einheitenmodell nutzen?

## Architekturentscheidungen

### AD-001 – C++20, Qt 6 und Open CASCADE Technology

Qt stellt die native Windows-Oberfläche, Internationalisierung und persistente
Einstellungen bereit. OCCT übernimmt exakte B-Rep-Geometrie, Visualisierung und
Datenaustausch. C++ vermeidet eine zusätzliche Laufzeitbrücke zwischen UI und
Geometriekern.

### AD-002 – Historienbasierter Feature-Baum

Skizzen und 3D-Operationen werden als versionierte Features mit stabiler ID und
Parametern im `.10cae`-Dokument gespeichert. Beim Öffnen sowie bei Undo/Redo wird
das Modell deterministisch in Reihenfolge des Feature-Baums neu berechnet.

### AD-003 – Trennung von Skizze, Feature und Darstellung

Skizzengeometrie und Abhängigkeiten sind dokumentunabhängige Datenmodelle. Der
Solver verändert dieses Modell; die UI visualisiert nur dessen Zustand. Aus gültigen
geschlossenen Profilen erzeugt der OCCT-Kern anschließend B-Rep-Features.

### AD-004 – Reproduzierbarer Abhängigkeitsaufbau

Qt und OCCT werden über ein fest gepinntes vcpkg-Manifest bezogen. Das portable
Paket enthält die benötigten Laufzeitbibliotheken und Lizenztexte.

## Stand 2026-10-15

Implementiert sind die native Ribbon-Shell, acht Arbeitsbereiche, DE/EN,
Dokumentspeicherung, Undo/Redo, STEP/STL-Export und ein interaktiver OCCT-Viewer.
Der erste parametrische Pfad speichert eine Rechteckskizze und berechnet daraus eine
Extrusion sowie nachgelagerte Fasen oder Rundungen. Die Neuberechnung erfolgt auch
nach Laden und Undo/Redo.

Die Geometrieoperationen werden durch Regressionstests validiert. Für einen
100-prozentig produktionsreifen CAD-Kern fehlen insbesondere ein vollständiger
Constraint-Solver, robuste persistente Topologiereferenzen, komplexe Profile,
Mehrkörperoperationen und Baugruppenbeziehungen.

## Validierungsergebnisse

| Datum | Prüfung | Ergebnis |
|---|---|---|
| 2026-10-15 | Dokument-Roundtrip und Schemafehler | bestanden |
| 2026-10-15 | Arbeitsbereichskatalog | bestanden |
| 2026-10-15 | Rechteckextrusion und Volumen | bestanden |
| 2026-10-15 | Fase und Rundung auf Testkörper | bestanden |
| 2026-10-15 | Skizzen-Solver: horizontal, lotrecht und koinzident | bestanden |
| 2026-10-15 | Skizzen-JSON-Roundtrip und geschlossene Profile | bestanden |
| 2026-10-15 | Erkennung widersprüchlicher Fixierungsabhängigkeiten | bestanden |
| 2026-10-15 | Rechteck per Zwei-Klick-Mausinteraktion | bestanden |
| 2026-10-15 | Escape-Abbruch und Löschen ausgewählter Skizzengeometrie | bestanden |
| 2026-10-15 | Extrusion gespeicherter Linien- und Kreisprofile | bestanden |
| 2026-10-15 | Skizzierebenen XY/XZ/YZ und ebenennormale Extrusion | bestanden |
| 2026-10-15 | Mittlere-Maustaste-Pan im Skizzeneditor | bestanden |
| 2026-10-15 | Längen-/Radiusmaß und sichtbare Constraint-Glyphen | bestanden |
| 2026-10-15 | Automatische Koinzidenz am Raster-Endpunkt | bestanden |
| 2026-10-15 | Punktabstands- und Winkelbedingungen inklusive JSON-Roundtrip | bestanden |
| 2026-10-15 | Drei-Punkt-Bogen, Endpunktfang und JSON-Roundtrip | bestanden |
| 2026-10-15 | Gemischtes Halbkreisprofil und analytischer Volumenvergleich | bestanden |
| 2026-10-15 | Bogenradius, Konzentrizität und Tangentialität | bestanden |
| 2026-10-15 | Typprüfung und Unterdrückung doppelter Constraints | bestanden |
| 2026-10-15 | Portabler Windows-x64-Start | bestanden |

## Versuch: erster Constraint-Solver

Der erste Solver arbeitet projektionsbasiert und iteriert geometrische Bedingungen
bis zu einer numerischen Toleranz. Linien besitzen vier, Kreise drei anfängliche
Freiheitsgrade. Koinzidenz entfernt zwei Freiheitsgrade; skalare Bedingungen wie
horizontal, vertikal, parallel, lotrecht, gleich, Länge oder Radius entfernen
jeweils einen. Fixierte Geometrie wird nicht verändert. Ein verbleibendes Residuum
oberhalb der Toleranz kennzeichnet einen Konflikt.

Dieses Verfahren ist für die nun implementierten elementaren Bedingungen
deterministisch und leicht testbar. Für gekoppelte große Skizzen wird später ein
Jacobi-/QR-basierter nichtlinearer Solver nötig; die öffentliche Datenstruktur bleibt
dabei erhalten.

## Versuch: interaktiver Skizzen-Canvas

Der Skizzeneditor verwendet eine von der 3D-Ansicht getrennte `QGraphicsView`.
Weltkoordinaten werden unabhängig von Zoom und Bildschirmauflösung gespeichert.
Rasterfang und Endpunktfang werden vor der Erzeugung einer Entität angewendet.
Rechtecke entstehen als vier einzelne Linien mit horizontalen und vertikalen
Abhängigkeiten; Kreise werden durch Mittelpunkt und Radius beschrieben.

Kritische Zustandsübergänge werden durch synthetische Qt-Ereignisse geprüft: zwei
Mausklicks schließen ein Rechteck ab, Escape verwirft eine angefangene Linie und
wechselt zum Auswahlwerkzeug, Delete entfernt die selektierte Entität mitsamt ihren
abhängigen Constraints. Der Test benötigt das Qt-Plattformplugin; dessen Pfad ist in
CTest explizit auf den manifestbasierten vcpkg-Baum gesetzt.

Geschlossene Linienprofile und einzelne Kreisprofile werden in planare OCCT-Flächen
umgewandelt und anschließend entlang der Z-Achse prismatisch extrudiert. Die Tests
vergleichen das resultierende Volumen mit dem analytischen Sollwert.

## Versuch: Ebenenabbildung und Bemaßung

Die zweidimensionalen Skizzenkoordinaten bleiben unabhängig von der gewählten
Modellebene. Erst beim Aufbau der OCCT-Kontur werden sie nach XY, XZ oder YZ
abgebildet; der Extrusionsvektor entspricht jeweils der positiven Ebenennormalen.
So bleibt der Constraint-Solver rein zweidimensional und benötigt keine drei
unterschiedlichen Rechenpfade.

Längenbedingungen skalieren den Richtungsvektor einer Linie bei festem Startpunkt,
Radiusbedingungen verändern den Kreisradius. Der Zahlenwert und geometrische
Constraints werden als transformationsunabhängige Glyphen dargestellt, damit sie
bei jeder Zoomstufe lesbar bleiben. Pan wird mit der mittleren Maustaste ausgeführt
und verändert ausschließlich den sichtbaren Ausschnitt, nicht die Modellkoordinaten.

## Versuch: automatische Koinzidenz und gekoppelte Maße

Beim Setzen eines Linienpunktes werden Raster und bestehende Endpunkte gemeinsam
bewertet. Bei identischer Distanz erhält ein vorhandener Endpunkt Vorrang vor dem
Raster. Dadurch entsteht zusätzlich zur identischen Anfangskoordinate eine explizite
Koinzidenzbedingung, die auch nach späteren Parameteränderungen erhalten bleibt.

Punktabstände beziehen sich explizit auf ausgewählte Endpunktindizes zweier
Entitäten. Winkelmaße speichern Gradwerte und drehen die zweite Linie relativ zur
ersten. Beide Constraint-Typen sind Bestandteil der JSON-Serialisierung und werden
bei jeder Neuberechnung erneut gelöst.

## Versuch: Drei-Punkt-Kreisbogen

Ein Kreisbogen wird durch Startpunkt, einen Punkt auf dem Bogen und Endpunkt
gespeichert. Diese Darstellung ist unmittelbar für die OCCT-Konstruktion mit
`GC_MakeArcOfCircle` geeignet und vermeidet Mehrdeutigkeiten bei großen Bögen.
Start- und Endpunkt nehmen am normalen Fang- und Koinzidenzsystem teil.

Die Bildschirmdarstellung berechnet aus den drei Punkten den Umkreis und wählt den
Bogenabschnitt, der den mittleren Punkt enthält. Damit stimmen Vorschau und exakte
B-Rep-Geometrie überein. Als Regressionstest wird ein Halbkreis mit Durchmesserlinie
geschlossen, extrudiert und gegen das analytische Volumen eines halben Zylinders
geprüft.

## Versuch: Kurvenabhängigkeiten

Für Kreise und Drei-Punkt-Bögen wird eine gemeinsame interne Kreisrepräsentation
aus Mittelpunkt und Radius berechnet. Eine Radiusbedingung skaliert beim Bogen alle
drei Definitionspunkte relativ zum unveränderten Umkreismittelpunkt. Konzentrizität
verschiebt die zweite Kurve, ohne ihren Radius oder Bogenwinkel zu verändern.

Tangentialität zwischen Linie und Kurve wird über den senkrechten Abstand des
Kreismittelpunkts zur unendlichen Linie gelöst. Bei zwei Kurven wird zunächst die
äußere Tangentialität mit einem Mittelpunktabstand entsprechend der Summe der Radien
verwendet. Innere Tangentialität wird später als eigener Modus ergänzt.

Vor dem Einfügen prüft das Modell die zulässigen Entitätstypen. Eine identische
Bedingung mit denselben Referenzen und demselben Wert liefert die vorhandene ID
zurück, statt den Freiheitsgrad ein zweites Mal zu reduzieren.

## Quellenbasis

- Autodesk Inventor 2026, Part Sketching:
  https://help.autodesk.com/cloudhelp/2026/ENU/Inventor-Help/files/GUID-97C8CDC5-AAAD-4A6F-BB84-24DB5C90C7A3.htm
- Autodesk Inventor 2026, Geometric Constraints:
  https://help.autodesk.com/cloudhelp/2026/ENU/Inventor-Help/files/GUID-A85A7A30-7D81-4C75-8769-CAD034EEA930.htm
- Autodesk Inventor 2026, Assembly Constraints:
  https://help.autodesk.com/cloudhelp/2026/ENU/Inventor-Help/files/GUID-CF4462AF-CEC6-4B8B-8F86-72D9AC9A3574.htm
- Open CASCADE Technology, Modeling Algorithms:
  https://dev.opencascade.org/doc/overview/html/occt_user_guides__modeling_algos.html

## Offene Risiken

- B-Rep-Kantenindizes sind nach Änderungen nicht stabil. Vor komplexen abhängigen
  Features wird daher ein semantisches Topologie-Namenssystem benötigt.
- Fasen und Rundungen können bei geometrisch unmöglichen Radien scheitern. Fehler
  müssen als Featurefehler sichtbar bleiben, ohne den letzten gültigen Körper zu
  zerstören.
- Ein eigener nichtlinearer Constraint-Solver benötigt numerische Toleranzen,
  Diagnose von Überbestimmung und reproduzierbare Lösungsstrategien.

