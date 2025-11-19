import os
import csv
import xml.etree.ElementTree as et
from collections import namedtuple
from typing import List

page_size = 30
call = namedtuple(
    "call", ["number", "duration", "readable_date", "type", "contact_name"]
)


def decode_type(t: str) -> str:
    """convert call type code to readable label"""
    return {"1": "incoming", "2": "outgoing", "3": "missed"}.get(t, "unknown")


def parse_file(path: str) -> List[call]:
    """parse xml file and return a list of call objects"""
    try:
        tree = et.parse(path)
        root = tree.getroot()
        calls = []
        for c in root.findall("call"):
            calls.append(
                call(
                    number=c.attrib.get("number", ""),
                    duration=c.attrib.get("duration", ""),
                    readable_date=c.attrib.get("readable_date", ""),
                    type=decode_type(c.attrib.get("type", "")),
                    contact_name=c.attrib.get("contact_name", ""),
                )
            )
        return calls
    except Exception as e:
        print(f"failed to parse xml: {e}")
        return []


def display_page(data: List[call], page: int) -> None:
    """print a unicode-styled table of call records (paged)"""
    os.system("cls" if os.name == "nt" else "clear")
    total_pages = (len(data) + page_size - 1) // page_size
    start = page * page_size
    end = start + page_size
    chunk = data[start:end]

    headers = ["#", "number", "duration", "date", "type", "contact name"]
    rows = [
        [str(i + start), c.number, c.duration, c.readable_date, c.type, c.contact_name]
        for i, c in enumerate(chunk)
    ]

    col_widths = [
        max(len(row[i]) for row in rows + [headers]) + 2 for i in range(len(headers))
    ]

    def draw_border(left: str, mid: str, right: str, fill: str = "─"):
        return left + mid.join(fill * w for w in col_widths) + right

    def draw_row(row, sep="│"):
        return (
            sep
            + sep.join(f" {cell:<{col_widths[i] - 2}} " for i, cell in enumerate(row))
            + sep
        )

    print(draw_border("╭", "┬", "╮"))
    print(draw_row(headers))
    print(draw_border("├", "┼", "┤"))
    for row in rows:
        print(draw_row(row))
    print(draw_border("╰", "┴", "╯"))

    print(f"page {page + 1} of {total_pages}\n")


def search_calls(data: List[call], term: str) -> List[call]:
    """filter calls by search term (case-insensitive, any field)"""
    term = term.lower()
    return [c for c in data if term in " ".join(c).lower()]


def sort_calls(data: List[call]) -> List[call]:
    """prompt for sort key and return sorted list"""
    keys = ["number", "duration", "readable_date", "type", "contact_name"]
    print(f'sort by: {", ".join(keys)}')
    key = input("key: ").strip()
    if key in keys:
        return sorted(data, key=lambda c: getattr(c, key).lower())
    else:
        print("invalid key")
        return data


def export_to_csv(data: List[call], filename: str = "export.csv") -> None:
    """export current call data to a csv file"""
    try:
        with open(filename, "w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow(call._fields)
            for c in data:
                writer.writerow(c)
        print(f"exported to {filename}")
    except Exception as e:
        print(f"failed to export: {e}")


def prompt_file() -> str:
    """ask user for a valid xml file path"""
    while True:
        path = input("enter path to xml file: ").strip()
        if not path:
            print("empty path")
        elif not os.path.exists(path):
            print("file does not exist")
        elif not path.lower().endswith(".xml"):
            print("file must be .xml")
        else:
            return path


def run_ui(data: List[call]) -> None:
    """main interactive loop for viewing/searching/sorting/exporting"""
    filtered = data
    page = 0

    while True:
        display_page(filtered, page)
        print("[n]ext [p]rev [s]earch [r]eset [o]rder [e]xport [q]uit")
        cmd = input("> ").strip().lower()

        if cmd == "n":
            if (page + 1) * page_size < len(filtered):
                page += 1
        elif cmd == "p":
            if page > 0:
                page -= 1
        elif cmd == "s":
            term = input("search term: ").strip()
            if term:
                filtered = search_calls(data, term)
                page = 0
        elif cmd == "r":
            filtered = data
            page = 0
        elif cmd == "o":
            filtered = sort_calls(filtered)
            page = 0
        elif cmd == "e":
            export_to_csv(filtered)
        elif cmd == "q":
            print("goodbye")
            break
        else:
            print("unknown command")


def main():
    print("call history viewer (xml)\n")
    path = prompt_file()
    data = parse_file(path)
    if not data:
        print("no data found")
        return
    run_ui(data)


if __name__ == "__main__":
    main()
