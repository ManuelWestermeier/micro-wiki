const fs = require("fs");

// "wissen.json" => wissen.bin (esp32 flash optimiert, sinnvolles Format für die Firmware)

// wissenjson jetzt:
/** {
 "geschichte": {
   "altertum": {
     "antike_hochkulturen": [
       {
         "typ": "H",
         "daten": "Fruehe Hochkulturen der Menschheit"
       },
       {
         "typ": "T",
         "daten": "Die ersten Hochkulturen entstanden unabhaengig voneinander in fruchtbaren Flussregionen - ein Muster das sich weltweit wiederholt."
       },
       {
         "typ": "LI",
         "daten": [
           "Mesopotamien: ca. 3500 v.Chr. - Tigris/Euphrat, Sumer, Keilschrift (aelteste Schrift!), Hammurabi-Codex (~1754 v.Chr.) - erstes Gesetzbuch",
           "Aegypten: ca. 3100 v.Chr. - Nil, Hieroglyphen, 3 Hauptperioden (Alt-, Mittel-, Neues Reich), Pyramiden (Cheops: 146m, ca. 2560 v.Chr.)",
           "Indus-Kultur: ca. 2600-1900 v.Chr. - Mohenjo-daro, Harappa, fortschrittliche Stadtplanung, Kanalisation",
           "Shang-Dynastie China: ab ca. 1600 v.Chr. - Orakelknochen (aelteste chin. Schrift), Bronze, Seidenstrasse beginnt",
           "Minoer (Kreta): ca. 2700-1450 v.Chr. - aelteste europaeische Hochkultur, Linear A (noch nicht entziffert)",
           "Olmeken (Mexiko): ca. 1500-400 v.Chr. - Kolossalkoepfe, Vorlaeufer der Maya"
         ]
       },
       {
         "typ": "H",
         "daten": "Antikes Griechenland - Wiege Europas"
       },
       {
         "typ": "LI",
         "daten": [
           "Archaische Zeit (800-500 v.Chr.): Polis-System, Olympische Spiele (776 v.Chr.), Homer (Ilias, Odyssee)",
           "Klassik (500-336 v.Chr.): Perserkriege, Marathonschlacht (490 v.Chr.), Peloponnesischer Krieg, Bluete Athens",
           "Attische Demokratie: Kleisthenes 508 v.Chr., Volksversammlung (Ekklesia), Ostrakismos (Scherbengericht)",
           "Sokrates (470-399): Hingerichtet wegen 'Gottlosigkeit', Dialektik",
           "Platon (428-348): Ideenlehre, Politeia (Idealstaat), Akademie",
           "Aristoteles (384-322): Schueler Platons, Lehrer Alexanders, Logik, Biologie, Physik, Poetik",
           "Alexander der Grosse (356-323): Weltreich von Griechenland bis Indien in 12 Jahren"
         ]
       },
       {
         "typ": "H",
         "daten": "Roemisches Reich - von der Stadt zur Weltmacht"
       },
       {
         "typ": "LI",
         "daten": [
           "Koenigszeit (753-509 v.Chr.): Romulus und Remus (Legende), 7 Koenige",
           "Republik (509-27 v.Chr.): Senat, Konsuln, Punische Kriege (264-146 v.Chr., Hannibal!), Gracchen, Caesar",
           "Julius Caesar: Gallischer Krieg, Rubikon (49 v.Chr.), Diktator, Iden des Maerz (44 v.Chr.)",
           "Kaiserzeit (27 v.Chr.-476 n.Chr.): Augustus => Pax Romana (200 Jahre Frieden)",
           "Hoehepunkt: Trajan, 117 n.Chr. - groesste Ausdehnung: ca. 5 Mio km^2",
           "Untergang: Voelkerwanderung, wirtschaftliche Krise, Teilung (285), 476 Romulus Augustulus abgesetzt"
         ]
       }
     ],
     "weitere_antike": [
       {
         "typ": "H",
         "daten": "Persisches Reich & Hellenismus"
       },
       {
         "typ": "LI",
         "daten": [
           "Achaemenidenreich: Kyros II. (559-530) => Darius => Xerxes | groesstes Reich seiner Zeit, ca. 5 Mio km^2",
           "Perserkriege: Marathon 490 v.Chr. (Botschaft: Laeufer stirbt nach 42km!) | Thermopylen 480 (300 Spartaner)",
           "Salamis 480 v.Chr.: griechische Flotte besiegt Perser | Themistokles' List | Wendepunkt",
           "Hellenismus (323-30 v.Chr.): nach Alexanders Tod | Diadochen teilen Weltreich | Verschmelzung grie.-orient. Kultur",
           "Bibliothek von Alexandria: bedeutendste Wissenssammlung der Antike | 400.000-700.000 Schriften | mehrfach beschaedigt",
           "Euklid (ca. 300 v.Chr.): Grundlagen der Geometrie | axiomatische Methode | 'Elemente' 2000 Jahre Standardwerk",
           "Archimedes (287-212 v.Chr.): Hebelgesetz, Auftrieb, PI-Annaeherung | 'Heureka!' | Syrakus",
           "Ptolemaeisches Weltbild: Erde im Zentrum | bis Kopernikus dominant | mathematisch funktionsfaehig!"
         ]
       },
       {
         "typ": "H",
         "daten": "Asiatische Hochkulturen"
       }, ......
        */

// es soll dafür optimiert sein:
// 1. UI select cathegory
// 2. select sub cathegory
// 3. select article
// 4. get article data