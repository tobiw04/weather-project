import os
import math

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
INPUT_FOLDER = os.path.join(SCRIPT_DIR, "data")
OUTPUT_FOLDER = os.path.join(SCRIPT_DIR, "output")

MEASUREMENTS = ["Jahr", "Monat", "Tag", "Stunde", "Minute", "air_temperature"]

os.makedirs(OUTPUT_FOLDER, exist_ok=True)


def get_column_indices(file_path):
    """
    Liest alle Headerzeilen bis '# begin of data' ein
    und gibt die Spaltennummern für die benötigten Messwerte zurück.
    """
    columns = {}
    header_lines = []

    with open(file_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if "begin of data" in line:
                break
            if line.startswith("#"):
                header_lines.append(line.lstrip("#").strip())

    # Jetzt header_lines durchgehen, um Spalten zu finden
    # Wir suchen die Zeilen, die air_temperature enthalten
    for hl in header_lines[::-1]:  # von unten nach oben
        parts = hl.split()
        for i, part in enumerate(parts):
            if part in MEASUREMENTS and part not in columns:
                columns[part] = i
        # air_temperature kann in mehreren Zeilen vorkommen → nehmen den ersten gefundenen Index
        if "air_temperature" in hl and "air_temperature" not in columns:
            columns["air_temperature"] = parts.index("air_temperature")

    return columns


def parse_file(path):
    """
    Extrahiert Jahr, Monat, Tag, Stunde, Minute, air_temperature
    aus der Datei.
    """
    results = []

    colmap = get_column_indices(path)

    # Prüfen, ob alle notwendigen Spalten gefunden wurden
    missing = [k for k in MEASUREMENTS if k not in colmap]
    if missing:
        print(f"Nicht alle benötigten Spalten gefunden! Fehlend: {missing}")
        return []

    # Datenzeilen lesen
    with open(path, "r", encoding="utf-8") as f:
        # bis Datenbereich springen
        for line in f:
            if "begin of data" in line:
                break

        # echte Daten lesen
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            try:
                # fix nan problem
                temp = float(parts[colmap["air_temperature"]])
                if not math.isfinite(temp):
                    continue

                row = [
                    int(parts[colmap["Jahr"]]),
                    int(parts[colmap["Monat"]]),
                    int(parts[colmap["Tag"]]),
                    int(parts[colmap["Stunde"]]),
                    int(parts[colmap["Minute"]]),
                    float(parts[colmap["air_temperature"]])
                ]
                results.append(row)
            except (IndexError, ValueError):
                continue  # Zeile überspringen, falls zu kurz oder falsches Format


    return results


def main():
    print("Suche Ordner:", INPUT_FOLDER)
    print("Existiert?", os.path.exists(INPUT_FOLDER))

    for filename in os.listdir(INPUT_FOLDER):
        if not filename.endswith(".csv"):
            continue

        in_path = os.path.join(INPUT_FOLDER, filename)
        print(f"Verarbeite {filename}")

        file_results = parse_file(in_path)
        if not file_results:
            print(f"Keine Daten extrahiert, Datei wird übersprungen.\n")
            continue

        # Output-Datei erzeugen
        out_name = filename.replace(".csv", "_out.csv")
        out_path = os.path.join(OUTPUT_FOLDER, out_name)

        with open(out_path, "w", encoding="utf-8") as out:
            # Kopfzeilen als Kommentare
            out.write("# Jahr \t Monat \t Tag \t Stunde \t Minute \t air_temperature\n")
            out.write("# begin of Data\n")

            # Datensätze schreiben
            for row in file_results:
                line = "\t".join([
                    str(row[0]),  # Jahr
                    str(row[1]),  # Monat
                    str(row[2]),  # Tag
                    str(row[3]),  # Stunde
                    str(row[4]),  # Minute
                    f"{row[5]:.2f}"  # air_temperature
                ])

                out.write(line + "\n")

        print(f"   ➜ gespeichert als {out_name} ({len(file_results)} Zeilen)\n")


if __name__ == "__main__":
    main()
